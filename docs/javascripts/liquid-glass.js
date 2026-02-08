/**
 * Dynamic Liquid Glass — mouse-tracking specular highlight
 *
 * Creates a radial-gradient light spot that follows the cursor across
 * glass-styled elements, giving them an interactive "liquid glass"
 * refraction/reflection feel inspired by Apple's Liquid Glass design.
 */
(function () {
  "use strict";

  var SELECTORS = [
    ".md-header",
    ".md-content",
    ".md-sidebar__scrollwrap",
    ".highlight",
    ".md-search__form",
    ".md-footer"
  ];

  /** Apply the radial highlight via CSS custom properties. */
  function applyHighlight(el, x, y) {
    var rect = el.getBoundingClientRect();
    var px = x - rect.left;
    var py = y - rect.top;
    el.style.setProperty("--glass-x", px + "px");
    el.style.setProperty("--glass-y", py + "px");
    el.classList.add("glass-active");
  }

  function clearHighlight(el) {
    el.classList.remove("glass-active");
  }

  /** Bind events once the DOM is ready. */
  function init() {
    var elements = [];
    SELECTORS.forEach(function (sel) {
      var nodes = document.querySelectorAll(sel);
      for (var i = 0; i < nodes.length; i++) {
        elements.push(nodes[i]);
      }
    });

    var ticking = false;
    document.addEventListener("mousemove", function (e) {
      if (ticking) return;
      ticking = true;
      requestAnimationFrame(function () {
        elements.forEach(function (el) {
          var rect = el.getBoundingClientRect();
          if (
            e.clientX >= rect.left &&
            e.clientX <= rect.right &&
            e.clientY >= rect.top &&
            e.clientY <= rect.bottom
          ) {
            applyHighlight(el, e.clientX, e.clientY);
          } else {
            clearHighlight(el);
          }
        });
        ticking = false;
      });
    });

    document.addEventListener("mouseleave", function () {
      elements.forEach(clearHighlight);
    });
  }

  if (document.readyState === "loading") {
    document.addEventListener("DOMContentLoaded", init);
  } else {
    init();
  }
})();
