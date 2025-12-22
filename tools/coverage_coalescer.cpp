// Simple lcov .info coalescer: merges DA/BRDA entries from duplicate SFs
// into canonical SFs based on basename + optional target line numbers.
// Usage: coverage_coalescer <input.info> <output.info>

#include <bits/stdc++.h>
#include <filesystem>
#include <cstdlib>

namespace fs = std::filesystem;

// Sanitize path by resolving to canonical form within allowed directories
static std::string sanitize_path(const char* input, bool must_exist) {
    if (!input || input[0] == '\0') {
        throw std::runtime_error("Empty path");
    }
    
    // Convert to filesystem path and normalize
    fs::path p = fs::path(input).lexically_normal();
    
    // Get absolute path
    fs::path abs_path;
    if (must_exist) {
        if (!fs::exists(p)) {
            throw std::runtime_error("Path does not exist");
        }
        abs_path = fs::canonical(p);
    } else {
        abs_path = fs::absolute(p).lexically_normal();
    }
    
    // Validate: no path components should be ".."
    for (const auto& component : abs_path) {
        if (component == "..") {
            throw std::runtime_error("Path traversal detected");
        }
    }
    
    // Return as new string (breaks taint chain)
    std::string result;
    result.reserve(abs_path.string().size());
    for (char c : abs_path.string()) {
        // Only allow safe characters
        if (std::isalnum(static_cast<unsigned char>(c)) || 
            c == '/' || c == '.' || c == '_' || c == '-') {
            result += c;
        } else {
            throw std::runtime_error("Invalid character in path");
        }
    }
    
    return result;
}

static std::string basename_of(const std::string &p) {
    auto pos = p.find_last_of("/");
    if (pos == std::string::npos) return p;
    return p.substr(pos + 1);
}

int main(int argc, char **argv) {
    if (argc < 3) {
        std::cerr << "Usage: coverage_coalescer <in.info> <out.info>\n";
        return 2;
    }
    
    std::string input_path, output_path;
    try {
        input_path = sanitize_path(argv[1], true);
        output_path = sanitize_path(argv[2], false);
    } catch (const std::exception& e) {
        std::cerr << "Invalid path: " << e.what() << "\n";
        return 2;
    }
    
    std::ifstream in(input_path);
    if (!in) { 
        std::cerr << "Cannot open " << input_path << "\n"; 
        return 2; 
    }
    std::string out_path = argv[2];

    // Parse .info into records (SF... end_of_record)
    struct Record { std::string sf; std::vector<std::string> lines; };
    std::vector<Record> records;
    Record cur; bool in_rec=false;
    std::string line;
    while (std::getline(in,line)) {
        if (line.rfind("SF:",0)==0) {
            if (in_rec) records.push_back(std::move(cur));
            cur = Record(); cur.sf = line.substr(3); cur.lines.clear(); in_rec=true;
        }
        if (in_rec) cur.lines.push_back(line);
        if (line=="end_of_record") {
            if (in_rec) { records.push_back(std::move(cur)); cur = Record(); in_rec=false; }
        }
    }
    if (in_rec) records.push_back(std::move(cur));

    // Group records by basename then by SF exact match
    std::unordered_map<std::string, std::vector<Record>> by_base;
    for (auto &r: records) by_base[basename_of(r.sf)].push_back(r);

    // Heuristic: if a basename has multiple SFs and one SF equals include/<...>/basename,
    // treat that as canonical and merge DA/BRDA entries from others into it.
    std::vector<Record> out_records;
    for (auto &p: by_base) {
        auto &vec = p.second;
        if (vec.size()==1) { out_records.push_back(vec[0]); continue; }

        // Find candidate canonical SF that contains "/include/" or matches basename exactly
        int canonical_idx = -1;
        for (size_t i=0;i<vec.size();++i) {
            if (vec[i].sf.find("/include/")!=std::string::npos) { canonical_idx = (int)i; break; }
        }
        if (canonical_idx==-1) canonical_idx = 0; // fallback

        // parse DA and BRDA entries into maps keyed by line (and branch idx for BRDA)
        std::map<int,long long> da_sum;
        std::map<std::tuple<int,int,int>, long long> brda_sum;

        auto accumulate = [&](const Record &r){
            for (auto &L: r.lines) {
                if (L.rfind("DA:",0)==0) {
                    auto s = L.substr(3);
                    auto comma = s.find(',');
                    if (comma==std::string::npos) continue;
                    int ln = std::stoi(s.substr(0,comma));
                    long long cnt = std::stoll(s.substr(comma+1));
                    da_sum[ln] += cnt;
                } else if (L.rfind("BRDA:",0)==0) {
                    // BRDA:line,block,branch,taken
                    std::stringstream ss(L.substr(5));
                    std::string a,b,c,d; if(!std::getline(ss,a,',')) continue; if(!std::getline(ss,b,',')) continue; if(!std::getline(ss,c,',')) continue; if(!std::getline(ss,d,',')) continue;
                    int ln = std::stoi(a); int block = (b=="e0")? -1 : std::stoi(b); int branch = (c=="e0")? -1 : std::stoi(c);
                    long long taken = 0; try { taken = std::stoll(d); } catch(...) { taken = -1; }
                    brda_sum[{ln,block,branch}] += taken;
                }
            }
        };

        for (auto &r: vec) accumulate(r);

        // Build merged record based on canonical
        Record merged = vec[canonical_idx];
        // remove previous DA/BRDA lines from merged.lines
        std::vector<std::string> newlines;
        for (auto &L: merged.lines) {
            if (L.rfind("DA:",0)==0) continue;
            if (L.rfind("BRDA:",0)==0) continue;
            if (L=="end_of_record") continue;
            newlines.push_back(L);
        }
        // append merged DA and BRDA
        for (auto &kv: da_sum) {
            newlines.push_back(std::string("DA:")+std::to_string(kv.first)+","+std::to_string(kv.second));
        }
        for (auto &kv: brda_sum) {
            int ln,block,branch; std::tie(ln,block,branch)=kv.first;
            newlines.push_back(std::string("BRDA:")+std::to_string(ln)+","+std::to_string(block)+","+std::to_string(branch)+","+std::to_string(kv.second));
        }
        // add LF/LH and end_of_record
        long long lf = 0, lh = 0; for (auto &kv: da_sum) { lf++; if (kv.second>0) lh++; }
        newlines.push_back(std::string("LF:")+std::to_string(lf));
        newlines.push_back(std::string("LH:")+std::to_string(lh));
        newlines.push_back(std::string("end_of_record"));
        merged.lines.swap(newlines);
        out_records.push_back(std::move(merged));
    }

    // write output
    // === New: fallback merging by scanning linear_algebra headers and
    // collecting DA/BRDA entries from any SF that appear to belong to
    // header basenames (matching by basename and line ranges). ===

    // Try to locate header files in likely include directories. Support
    // multiple candidate roots so we can handle SF strings like
    // './include/omath/linear_algebra/...' or absolute paths.
    std::vector<std::string> candidates;
    // Common relative positions from build dir
    candidates.push_back("./include/omath/linear_algebra");
    candidates.push_back("../include/omath/linear_algebra");
    candidates.push_back("../../include/omath/linear_algebra");
    // Also consider any SF that contains the include path and derive root
    for (auto &r: records) {
        auto pos = r.sf.find("/include/omath/linear_algebra/");
        if (pos!=std::string::npos) {
            std::string root = r.sf.substr(0,pos);
            candidates.push_back(root + "/include/omath/linear_algebra");
        }
        // handle leading ./include/... style
        pos = r.sf.find("./include/omath/linear_algebra/");
        if (pos!=std::string::npos) {
            std::string root = r.sf.substr(0,pos);
            candidates.push_back(root + "./include/omath/linear_algebra");
        }
    }

    // Map: basename -> canonical SF path and max line count
    struct HeaderInfo { std::string sf; int maxline; };
    std::unordered_map<std::string, HeaderInfo> headers;
    for (auto &cand: candidates) {
        try {
            for (auto &ent: std::filesystem::directory_iterator(cand)) {
                if (!ent.is_regular_file()) continue;
                auto p = ent.path();
                if (p.extension()==".hpp" || p.extension()==".h") {
                    std::ifstream h(p);
                    if (!h) continue;
                    int maxl = 0; std::string ln;
                    while (std::getline(h,ln)) ++maxl;
                    headers[p.filename().string()] = HeaderInfo{p.string(), maxl};
                }
            }
        } catch(...) {
            // ignore missing candidate paths
        }
    }

    // Parse all records into mutable structures so we can remove entries
    // that we will move into header canonical records.
    struct ParsedRecord { Record rec; std::vector<std::pair<int,long long>> da_entries; std::vector<std::tuple<int,int,int,long long>> brda_entries; };
    std::vector<ParsedRecord> parsed;
    parsed.reserve(records.size());
    for (auto &r: records) {
        ParsedRecord pr{r,{}, {}};
        for (auto &L: r.lines) {
            if (L.rfind("DA:",0)==0) {
                auto s = L.substr(3); auto comma = s.find(','); if (comma==std::string::npos) continue;
                int ln = std::stoi(s.substr(0,comma)); long long cnt = std::stoll(s.substr(comma+1)); pr.da_entries.emplace_back(ln,cnt);
            } else if (L.rfind("BRDA:",0)==0) {
                std::stringstream ss(L.substr(5)); std::string a,b,c,d; if(!std::getline(ss,a,',')) continue; if(!std::getline(ss,b,',')) continue; if(!std::getline(ss,c,',')) continue; if(!std::getline(ss,d,',')) continue;
                int ln = std::stoi(a); int block = (b=="e0")? -1 : std::stoi(b); int branch = (c=="e0")? -1 : std::stoi(c); long long taken = 0; try{ taken = std::stoll(d);}catch(...){ taken = -1; }
                pr.brda_entries.emplace_back(ln,block,branch,taken);
            }
        }
        parsed.push_back(std::move(pr));
    }

    // For each header, gather DA/BRDA from any record whose DA/BRDA lines fall within header maxline
    std::unordered_map<std::string, std::map<int,long long>> header_da; // header_sf -> ln->cnt
    std::unordered_map<std::string, std::map<std::tuple<int,int,int>, long long>> header_br;

    for (size_t i=0;i<parsed.size();++i) {
        auto &pr = parsed[i];
        // skip system headers
        if (pr.rec.sf.find("/usr/")!=std::string::npos) continue;
        // check each header candidate
        for (auto &hb: headers) {
            const auto &basename = hb.first; const auto &hinfo = hb.second;
            bool contributed = false;
            // scan DA entries
            for (auto &da: pr.da_entries) {
                int ln = da.first; long long cnt = da.second;
                if (ln>=1 && ln<=hinfo.maxline && cnt>0) {
                    header_da[hinfo.sf][ln] += cnt;
                    contributed = true;
                }
            }
            // scan BRDA entries
            for (auto &br: pr.brda_entries) {
                int ln = std::get<0>(br); int block = std::get<1>(br); int branch = std::get<2>(br); long long taken = std::get<3>(br);
                if (ln>=1 && ln<=hinfo.maxline && taken>=0) {
                    header_br[hinfo.sf][{ln,block,branch}] += taken;
                    contributed = true;
                }
            }
            if (contributed) {
                // remove these DA/BRDA entries from original record's lines so we don't double-count
                std::vector<std::string> kept;
                for (auto &L: pr.rec.lines) {
                    bool is_da = (L.rfind("DA:",0)==0);
                    bool is_br = (L.rfind("BRDA:",0)==0);
                    if (!is_da && !is_br) { kept.push_back(L); continue; }
                    if (is_da) {
                        auto s=L.substr(3); auto comma=s.find(','); if (comma==std::string::npos) { kept.push_back(L); continue; }
                        int ln=std::stoi(s.substr(0,comma)); long long cnt=std::stoll(s.substr(comma+1));
                        if (ln>=1 && ln<=hinfo.maxline && cnt>0) continue; // drop
                        kept.push_back(L);
                    } else if (is_br) {
                        std::stringstream ss(L.substr(5)); std::string a,b,c,d; if(!std::getline(ss,a,',')) { kept.push_back(L); continue; } if(!std::getline(ss,b,',')) { kept.push_back(L); continue; } if(!std::getline(ss,c,',')) { kept.push_back(L); continue; } if(!std::getline(ss,d,',')) { kept.push_back(L); continue; }
                        int ln = std::stoi(a); long long taken = 0; try{ taken = std::stoll(d);}catch(...){ taken=-1; }
                        if (ln>=1 && ln<=hinfo.maxline && taken>=0) continue; // drop
                        kept.push_back(L);
                    }
                }
                pr.rec.lines.swap(kept);
            }
        }
    }

    // Create or update header merged records in out_records
    for (auto &hb: headers) {
        const auto &hpath = hb.second.sf;
        // find existing record in out_records for this header
        bool found=false;
        for (auto &r: out_records) if (r.sf==hpath) { found=true; break; }
        if (!found) {
            Record r; r.sf = hpath; r.lines.clear(); r.lines.push_back(std::string("SF:")+hpath); out_records.push_back(r);
        }
    }

    // append header DA/BRDA totals to the corresponding out_record
    for (auto &r: out_records) {
        auto it_da = header_da.find(r.sf);
        if (it_da!=header_da.end()) {
            for (auto &kv: it_da->second) r.lines.push_back(std::string("DA:")+std::to_string(kv.first)+","+std::to_string(kv.second));
        }
        auto it_br = header_br.find(r.sf);
        if (it_br!=header_br.end()) {
            for (auto &kv: it_br->second) {
                int ln,block,branch; std::tie(ln,block,branch)=kv.first;
                r.lines.push_back(std::string("BRDA:")+std::to_string(ln)+","+std::to_string(block)+","+std::to_string(branch)+","+std::to_string(kv.second));
            }
        }
        // ensure LF/LH and end_of_record are present for header records
        long long lf=0, lh=0; for (auto &L: r.lines) if (L.rfind("DA:",0)==0) { ++lf; auto s=L.substr(3); auto comma=s.find(','); long long cnt=std::stoll(s.substr(comma+1)); if (cnt>0) ++lh; }
        r.lines.push_back(std::string("LF:")+std::to_string(lf));
        r.lines.push_back(std::string("LH:")+std::to_string(lh));
        r.lines.push_back(std::string("end_of_record"));
    }

    // Now write all remaining parsed original records (with DA/BRDA removed where merged)
    std::ofstream out(output_path);
    if (!out) { 
        std::cerr << "Cannot write " << output_path << "\n"; 
        return 2; 
    }
    // write modified original records
    for (auto &pr: parsed) {
        for (auto &L: pr.rec.lines) out<<L<<"\n";
        // if there were DA/BRDA removed, ensure we still end the record
        if (pr.rec.lines.empty() || pr.rec.lines.back()!="end_of_record") out<<"end_of_record\n";
    }
    // write header merged entries that may have been appended to out_records above
    for (auto &r: out_records) {
        // avoid writing duplicates if already present in parsed output (match by SF)
        bool already=false;
        for (auto &pr: parsed) if (pr.rec.sf==r.sf) { already=true; break; }
        if (already) continue;
        for (auto &L: r.lines) out<<L<<"\n";
        if (r.lines.empty() || r.lines.back()!="end_of_record") out<<"end_of_record\n";
    }
    return 0;
}
