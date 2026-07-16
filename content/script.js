/* ==========================================================================
   PR1103 HTTP Server — Frontend Logic
   - Server proof: live header fetching
   - Playground API calls
   - Scroll-reveal animations
   ========================================================================== */

(function () {
    "use strict";

    // ---- Scroll Reveal ----
    const revealElements = document.querySelectorAll(
        ".arch-card, .method-row, .playground-card"
    );

    const revealObserver = new IntersectionObserver(
        (entries) => {
            entries.forEach((entry) => {
                if (entry.isIntersecting) {
                    const siblings = Array.from(entry.target.parentElement.children);
                    const index = siblings.indexOf(entry.target);
                    entry.target.style.transitionDelay = `${index * 80}ms`;
                    entry.target.classList.add("visible");
                    revealObserver.unobserve(entry.target);
                }
            });
        },
        { threshold: 0.15, rootMargin: "0px 0px -40px 0px" }
    );

    revealElements.forEach((el) => revealObserver.observe(el));



    // ---- Playground API Calls ----

    function escapeHtml(str) {
        const div = document.createElement("div");
        div.textContent = str;
        return div.innerHTML;
    }

    function showResponse(containerId, status, body, isOk) {
        const container = document.getElementById(containerId);
        const statusEl = container.querySelector(".res-status");
        const bodyEl = container.querySelector(".res-body");

        statusEl.textContent = `HTTP ${status}`;
        statusEl.className = isOk ? "res-status res-status--ok" : "res-status res-status--err";

        // Pretty-print JSON if possible
        try {
            const parsed = JSON.parse(body);
            bodyEl.textContent = JSON.stringify(parsed, null, 2);
        } catch {
            bodyEl.textContent = body;
        }

        container.classList.add("visible");
    }

    async function apiCall(url, options, containerId, btnEl) {
        btnEl.classList.add("play-btn--loading");
        btnEl.textContent = "Sending...";

        try {
            const start = performance.now();
            const res = await fetch(url, options);
            const elapsed = (performance.now() - start).toFixed(1);
            const text = await res.text();

            showResponse(
                containerId,
                `${res.status} (${elapsed}ms)`,
                text,
                res.ok
            );
        } catch (err) {
            showResponse(containerId, "Error", err.message, false);
        } finally {
            btnEl.classList.remove("play-btn--loading");
            btnEl.innerHTML = `Send Request <svg width="14" height="14" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2.5" stroke-linecap="round" stroke-linejoin="round"><line x1="5" y1="12" x2="19" y2="12"></line><polyline points="12 5 19 12 12 19"></polyline></svg>`;
        }
    }

    // Bind playground buttons
    document.getElementById("btnPing").addEventListener("click", function () {
        apiCall("/api/ping", {}, "resPing", this);
    });

    document.getElementById("btnServerInfo").addEventListener("click", function () {
        apiCall("/api/server-info", {}, "resServerInfo", this);
    });

    document.getElementById("btnEcho").addEventListener("click", function () {
        const body = document.getElementById("echoInput").value || '{"test": true}';
        apiCall(
            "/api/echo",
            {
                method: "POST",
                headers: { "Content-Type": "application/json" },
                body: body,
            },
            "resEcho",
            this
        );
    });

    // ---- Benchmark Tabs ----
    const benchTabs = document.querySelectorAll(".bench-tab");
    const benchPanels = document.querySelectorAll(".bench-panel");

    benchTabs.forEach((tab) => {
        tab.addEventListener("click", () => {
            const level = tab.dataset.level;

            benchTabs.forEach((t) => t.classList.remove("bench-tab--active"));
            benchPanels.forEach((p) => p.classList.remove("bench-panel--active"));

            tab.classList.add("bench-tab--active");
            const panel = document.querySelector(`.bench-panel[data-level="${level}"]`);
            if (panel) panel.classList.add("bench-panel--active");
        });
    });

    // ---- Smooth scroll for nav links ----
    document.querySelectorAll('a[href^="#"]').forEach((link) => {
        link.addEventListener("click", (e) => {
            const target = document.querySelector(link.getAttribute("href"));
            if (target) {
                e.preventDefault();
                const offset = 33 + 56 + 24; // banner + nav + padding
                const y = target.getBoundingClientRect().top + window.scrollY - offset;
                window.scrollTo({ top: y, behavior: "smooth" });
            }
        });
    });
})();
