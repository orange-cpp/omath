document.addEventListener('DOMContentLoaded', function(){
  if (window.hljs) {
    // register any languages if needed by CDN includes
    try { hljs.highlightAll(); } catch(e) { console.warn('hljs highlightAll failed', e); }
  }
});
