/* ==========================================================================
   PR1103 HTTP Server — Frontend Logic
   - Playground API calls
   - Scroll-reveal animations
   - Terminal typewriter effect
   ========================================================================== */

(function () {
    "use strict";

    // ---- Scroll Reveal ----
    const revealElements = document.querySelectorAll(
        ".arch-card, .method-row, .playground-card, .bench-card"
    );

    const revealObserver = new IntersectionObserver(
        (entries) => {
            entries.forEach((entry, i) => {
                if (entry.isIntersecting) {
                    // Stagger each element's appearance within its batch
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

    // ---- Terminal Typewriter ----
    const terminalLines = [
        { type: "cmd", text: "$ curl -s http://localhost:3490/api/ping" },
        { type: "out", text: '{"status":"ok"}' },
        { type: "success", text: "Connection from 127.0.0.1" },
        { type: "out", text: "Method: GET  Path: /api/ping  Version: HTTP/1.1" },
        { type: "cmd", text: '$ curl -s http://localhost:3490/api/hello' },
        { type: "out", text: '{"message":"hello world"}' },
        { type: "success", text: "Connection from 127.0.0.1" },
        { type: "out", text: "Method: GET  Path: /api/hello  Version: HTTP/1.1" },
    ];

    const termBody = document.getElementById("terminalBody");
    let termLineIndex = 0;

    function appendTerminalLine() {
        if (termLineIndex >= terminalLines.length) {
            // Loop back after a long pause
            setTimeout(() => {
                // Remove the appended lines and restart
                const appended = termBody.querySelectorAll(".term-line--dynamic");
                appended.forEach((el) => el.remove());
                termLineIndex = 0;
                setTimeout(appendTerminalLine, 1500);
            }, 6000);
            return;
        }

        const line = terminalLines[termLineIndex];
        const div = document.createElement("div");
        div.className = "term-line term-line--dynamic";

        if (line.type === "cmd") {
            div.innerHTML = `<span class="term-prompt">$</span> <span class="term-cmd">${escapeHtml(
                line.text.substring(2)
            )}</span>`;
        } else if (line.type === "success") {
            div.className += " term-output term-success";
            div.textContent = line.text;
        } else {
            div.className += " term-output";
            div.textContent = line.text;
        }

        div.style.opacity = "0";
        div.style.transform = "translateY(4px)";
        termBody.appendChild(div);
        termBody.scrollTop = termBody.scrollHeight;

        // Animate in
        requestAnimationFrame(() => {
            div.style.transition = "opacity 0.3s ease, transform 0.3s ease";
            div.style.opacity = "1";
            div.style.transform = "translateY(0)";
        });

        termLineIndex++;
        const delay = line.type === "cmd" ? 1200 : 400;
        setTimeout(appendTerminalLine, delay);
    }

    // Start typewriter after a short delay
    setTimeout(appendTerminalLine, 2500);

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

    document.getElementById("btnHello").addEventListener("click", function () {
        apiCall("/api/hello", {}, "resHello", this);
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
