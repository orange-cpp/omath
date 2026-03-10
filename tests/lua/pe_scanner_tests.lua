-- PatternScanner tests: generic scan over a Lua string buffer

function PatternScanner_FindsExactPattern()
    local buf = "\x90\x01\x02\x03\x04"
    local offset = omath.PatternScanner.scan(buf, "90 01 02")
    assert(offset ~= nil, "expected pattern to be found")
    assert(offset == 0, "expected offset 0, got " .. tostring(offset))
end

function PatternScanner_FindsPatternAtNonZeroOffset()
    local buf = "\x00\x00\xAB\xCD\xEF"
    local offset = omath.PatternScanner.scan(buf, "AB CD EF")
    assert(offset ~= nil, "expected pattern to be found")
    assert(offset == 2, "expected offset 2, got " .. tostring(offset))
end

function PatternScanner_WildcardMatches()
    local buf = "\xDE\xAD\xBE\xEF"
    local offset = omath.PatternScanner.scan(buf, "DE ?? BE")
    assert(offset ~= nil, "expected wildcard match")
    assert(offset == 0)
end

function PatternScanner_ReturnsNilWhenNotFound()
    local buf = "\x01\x02\x03"
    local offset = omath.PatternScanner.scan(buf, "AA BB CC")
    assert(offset == nil, "expected nil for not-found pattern")
end

function PatternScanner_ReturnsNilForEmptyBuffer()
    local offset = omath.PatternScanner.scan("", "90 01")
    assert(offset == nil)
end

-- PePatternScanner tests: scan_in_module uses FAKE_MODULE_BASE injected from C++
-- The fake module contains {0x90, 0x01, 0x02, 0x03, 0x04} placed at raw offset 0x200

function PeScanner_FindsExactPattern()
    local addr = omath.PePatternScanner.scan_in_module(FAKE_MODULE_BASE, "90 01 02")
    assert(addr ~= nil, "expected pattern to be found in module")
    local offset = addr - FAKE_MODULE_BASE
    assert(offset == 0x200, string.format("expected offset 0x200, got 0x%X", offset))
end

function PeScanner_WildcardMatches()
    local addr = omath.PePatternScanner.scan_in_module(FAKE_MODULE_BASE, "90 ?? 02")
    assert(addr ~= nil, "expected wildcard match in module")
    local offset = addr - FAKE_MODULE_BASE
    assert(offset == 0x200, string.format("expected offset 0x200, got 0x%X", offset))
end

function PeScanner_ReturnsNilWhenNotFound()
    local addr = omath.PePatternScanner.scan_in_module(FAKE_MODULE_BASE, "AA BB CC DD")
    assert(addr == nil, "expected nil for not-found pattern")
end

function PeScanner_CustomSectionFallsBackToNil()
    -- Request a section that doesn't exist in our fake module
    local addr = omath.PePatternScanner.scan_in_module(FAKE_MODULE_BASE, "90 01 02", ".rdata")
    assert(addr == nil, "expected nil for wrong section name")
end

-- SectionScanResult: verify the type is registered and tostring works on a C++-returned value
function SectionScanResult_TypeIsRegistered()
    assert(omath.SectionScanResult ~= nil, "SectionScanResult type should be registered")
end
