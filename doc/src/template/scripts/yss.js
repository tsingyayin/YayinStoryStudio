/*
	Yayin Story Studio 项目文档：交互脚本

	全部是渐进增强，脚本失效时页面依然可读：
	1. 深浅色切换（记录在 localStorage，首次跟随系统；支持时用圆形扩散动画过渡）；
	2. 窄屏下把侧边目录折叠成按钮开关；
	3. 滚动时高亮侧边目录中当前所在的章节，指示块跟随滑动；
	4. 给代码块加复制按钮、给标题加锚点链接；
	5. 把正文里 QDoc 没列进目录的章节（成员函数文档等）补进侧边目录；
	6. 顶栏下沿的阅读进度条、滚动时顶栏自动收放、回到顶部按钮；
	7. 正文元素滚入视口时的渐显（首屏内容不参与，避免闪一下再动）；
	8. 背景点阵与鼠标涟漪（canvas 绘制，色晕只是 CSS 上的一层固定淡底）。
*/

(function () {
	"use strict";

	var THEME_KEY = "yss-doc-theme";
	// 主题变化时要跟着更新的东西（目前只有背景点阵的颜色）
	var themeListeners = [];

	function onThemeChange(fn) {
		themeListeners.push(fn);
	}

	function each(list, fn) {
		Array.prototype.forEach.call(list, fn);
	}

	/* ----------------------------------------------------------
		1. 深浅色
		---------------------------------------------------------- */

	function currentTheme() {
		var explicit = document.documentElement.getAttribute("data-theme");
		if (explicit === "light" || explicit === "dark") {
			return explicit;
		}
		return window.matchMedia("(prefers-color-scheme: dark)").matches ? "dark" : "light";
	}

	function applyTheme(theme, persist) {
		document.documentElement.setAttribute("data-theme", theme);
		if (persist) {
			try {
				localStorage.setItem(THEME_KEY, theme);
			} catch (e) {
				/* 隐私模式下忽略写入失败 */
			}
		}
		updateThemeButton(theme);
		// 通知监听者。注意：这里刚改完属性，计算样式可能还没重算，
		// 所以监听者应该把取色动作放到下一帧（点阵就是这么做的）。
		each(themeListeners, function (fn) {
			fn();
		});
	}

	function updateThemeButton(theme) {
		var button = document.getElementById("yss-theme-toggle");
		if (!button) {
			return;
		}
		button.textContent = theme === "dark" ? "☀" : "☾";
		button.setAttribute("title", theme === "dark" ? "切换到浅色" : "切换到深色");
		button.setAttribute("aria-label", button.getAttribute("title"));
	}

	function initTheme() {
		var button = document.getElementById("yss-theme-toggle");
		updateThemeButton(currentTheme());
		if (!button) {
			return;
		}
		button.addEventListener("click", function () {
			switchTheme(button);
		});
	}

	// 切换主题。支持 View Transition 的浏览器里，新配色以按钮为圆心
	// 圆形扩散揭开（配合 CSS 里的 ::view-transition-new 关键帧），
	// 否则退化成直接切换。系统开了“减少动态效果”时也不做过渡。
	function switchTheme(button) {
		var next = currentTheme() === "dark" ? "light" : "dark";
		function apply() {
			applyTheme(next, true);
		}
		if (document.startViewTransition && !prefersReduced()) {
			var rect = button.getBoundingClientRect();
			document.documentElement.style.setProperty("--yss-vt-x", (rect.left + rect.width / 2) + "px");
			document.documentElement.style.setProperty("--yss-vt-y", (rect.top + rect.height / 2) + "px");
			document.startViewTransition(apply);
			return;
		}
		apply();
	}

	function prefersReduced() {
		return window.matchMedia && window.matchMedia("(prefers-reduced-motion: reduce)").matches;
	}

	/* ----------------------------------------------------------
		2. 窄屏目录开关
		---------------------------------------------------------- */

	function initMenu() {
		var button = document.getElementById("yss-menu-toggle");
		var sidebar = document.querySelector(".sidebar");
		if (!button || !sidebar) {
			return;
		}
		button.addEventListener("click", function () {
			sidebar.classList.toggle("is-open");
			button.setAttribute("aria-expanded", sidebar.classList.contains("is-open") ? "true" : "false");
		});
	}

	/* ----------------------------------------------------------
		3. 目录滚动高亮
		---------------------------------------------------------- */

	function initScrollSpy() {
		var links = document.querySelectorAll(".toc a[href^='#']");
		if (!links.length || !("IntersectionObserver" in window)) {
			return;
		}

		var byId = {};
		var targets = [];
		each(links, function (link) {
			var id = decodeURIComponent(link.getAttribute("href").slice(1));
			var heading = id ? document.getElementById(id) : null;
			if (heading) {
				byId[id] = link;
				targets.push(heading);
			}
		});
		if (!targets.length) {
			return;
		}

		// 活动指示块：一个跟着当前章节滑动的方块，比每项自己换背景色顺眼
		var toc = document.querySelector(".toc");
		var marker = null;
		var firstMove = true;
		if (toc) {
			marker = document.createElement("span");
			marker.className = "toc-marker";
			marker.setAttribute("aria-hidden", "true");
			toc.appendChild(marker);
		}

		function moveMarker(link) {
			// 目录被折叠（窄屏未展开）时 offset 全是 0，此时不动它
			if (!marker || !link || !link.offsetParent) {
				return;
			}
			marker.style.width = link.offsetWidth + "px";
			marker.style.height = link.offsetHeight + "px";
			marker.style.transform = "translate(" + link.offsetLeft + "px," + link.offsetTop + "px)";
			marker.classList.add("is-ready");
			if (firstMove) {
				// 首次定位不要从左上角滑过去，直接落位
				marker.style.transition = "none";
				void marker.offsetWidth;
				window.requestAnimationFrame(function () {
					marker.style.transition = "";
				});
				firstMove = false;
			}
		}

		var activeId = null;

		function highlight(id) {
			if (id === activeId) {
				return;
			}
			activeId = id;
			each(links, function (link) {
				link.classList.remove("is-active");
			});
			if (byId[id]) {
				byId[id].classList.add("is-active");
				moveMarker(byId[id]);
			}
		}

		// 侧边栏尺寸变化（窄屏抽屉展开、窗口缩放）后重新落位
		window.addEventListener("resize", function () {
			if (activeId && byId[activeId]) {
				moveMarker(byId[activeId]);
			}
		});
		var sidebar = document.querySelector(".sidebar");
		if (sidebar && window.ResizeObserver) {
			new ResizeObserver(function () {
				if (activeId && byId[activeId]) {
					moveMarker(byId[activeId]);
				}
			}).observe(sidebar);
		}

		var visible = {};
		var observer = new IntersectionObserver(
			function (entries) {
				entries.forEach(function (entry) {
					if (entry.isIntersecting) {
						visible[entry.target.id] = true;
					} else {
						delete visible[entry.target.id];
					}
				});
				// 取文档顺序里最靠前的可见标题
				for (var i = 0; i < targets.length; i++) {
					if (visible[targets[i].id]) {
						highlight(targets[i].id);
						return;
					}
				}
			},
			{ rootMargin: "-" + (64 + 12) + "px 0px -70% 0px", threshold: 0 }
		);
		targets.forEach(function (target) {
			observer.observe(target);
		});
	}

	/* ----------------------------------------------------------
		4. 代码复制按钮与标题锚点
		---------------------------------------------------------- */

	function initCodeCopy() {
		each(document.querySelectorAll("pre"), function (pre) {
			if (pre.querySelector(".yss-copy")) {
				return;
			}
			var button = document.createElement("button");
			button.type = "button";
			button.className = "yss-copy";
			button.textContent = "复制";
			button.addEventListener("click", function () {
				var text = pre.innerText;
				function done() {
					button.textContent = "已复制";
					window.setTimeout(function () {
						button.textContent = "复制";
					}, 1500);
				}
				if (navigator.clipboard && navigator.clipboard.writeText) {
					navigator.clipboard.writeText(text).then(done, function () {
						button.textContent = "复制失败";
					});
					return;
				}
				// 旧环境回退
				var area = document.createElement("textarea");
				area.value = text;
				area.setAttribute("readonly", "");
				area.style.position = "fixed";
				area.style.left = "-9999px";
				document.body.appendChild(area);
				area.select();
				try {
					document.execCommand("copy");
					done();
				} catch (e) {
					button.textContent = "复制失败";
				}
				document.body.removeChild(area);
			});
			pre.appendChild(button);
		});
	}

	function initHeadingAnchors() {
		each(document.querySelectorAll(".context h2[id], .context h3[id]"), function (heading) {
			if (heading.querySelector(".yss-anchor")) {
				return;
			}
			var anchor = document.createElement("a");
			anchor.className = "yss-anchor";
			anchor.href = "#" + heading.id;
			anchor.textContent = "#";
			anchor.setAttribute("aria-label", "本节链接");
			heading.insertBefore(anchor, heading.firstChild);
		});
	}

	/* ----------------------------------------------------------
		5. 顶栏里标出当前模块
		---------------------------------------------------------- */

	function initCurrentNav() {
		var page = location.pathname.split("/").pop() || "index.html";
		var module = page.replace(/-module\.html$/, "");
		each(document.querySelectorAll(".yss-nav a"), function (link) {
			var href = link.getAttribute("href");
			if (href === page) {
				link.classList.add("is-current");
				return;
			}
			if (href && href.indexOf(module + "-module.html") === 0 && module) {
				link.classList.add("is-current");
			}
		});
	}

	/* ----------------------------------------------------------
		6. 把正文里缺的章节补进侧边目录

		QDoc 生成的目录只含它自己的章节（公开成员函数、详细说明……），
		“详细说明”里的分组标题（成员函数文档 / 成员类型文档 / 宏文档）
		既没有 id 也不在目录里，这里把它们补上。
		---------------------------------------------------------- */

	function isBefore(a, b) {
		return !!(a.compareDocumentPosition(b) & Node.DOCUMENT_POSITION_FOLLOWING);
	}

	function initTocExtra() {
		var list = document.querySelector(".toc ul");
		if (!list || !window.Node) {
			return;
		}

		var known = {};
		var labels = {};
		var items = [];
		each(list.querySelectorAll("a[href^='#']"), function (a, i) {
			known[decodeURIComponent(a.getAttribute("href").slice(1))] = true;
			labels[a.textContent.replace(/\s+/g, "")] = true;
			if (i < list.children.length && list.children[i].tagName === "LI") {
				items.push(list.children[i]);
			}
		});

		var add = [];
		var seq = 0;
		each(document.querySelectorAll(".context h2, .context h3"), function (heading) {
			var label = heading.textContent.replace(/\s+/g, " ").trim();
			// 函数签名（h3.fn）、枚举/flags 签名（h3.flags，带 [since …] 标记与 .name）、
			// 表格内部、已有条目、重名条目，都不进目录
			if (
				!label ||
				heading.classList.contains("fn") ||
				heading.classList.contains("flags") ||
				heading.querySelector("code.details.extra, code.summary.extra, .name, .type") ||
				heading.closest(".table")
			) {
				return;
			}
			if (heading.id && known[heading.id]) {
				return;
			}
			if (labels[label.replace(/\s+/g, "")]) {
				return;
			}
			if (!heading.id) {
				seq++;
				heading.id = "yss-sec-" + seq;
			}
			known[heading.id] = true;
			labels[label.replace(/\s+/g, "")] = true;
			add.push({ heading: heading, label: label });
		});
		if (!add.length) {
			return;
		}

		// 按正文顺序插入：找到第一个排在它后面的原目录项，插到那一项前面。
		// 连续多条命中同一个锚点时，因为总是插在锚点前，先后的相对顺序仍然正确。
		each(add, function (entry) {
			var li = document.createElement("li");
			// 与 QDoc 的映射保持一致：h2 -> level1，h3 -> level2
			li.className = entry.heading.tagName === "H2" ? "level1" : "level2";
			var link = document.createElement("a");
			link.href = "#" + entry.heading.id;
			link.textContent = entry.label;
			li.appendChild(link);

			for (var i = 0; i < items.length; i++) {
				var href = items[i].querySelector("a[href^='#']");
				var target = href
					? document.getElementById(decodeURIComponent(href.getAttribute("href").slice(1)))
					: null;
				if (target && isBefore(entry.heading, target)) {
					list.insertBefore(li, items[i]);
					return;
				}
			}
			list.appendChild(li);
		});
	}

	/* ----------------------------------------------------------
		7. 滚动观感：阅读进度条、顶栏收放、回到顶部
		---------------------------------------------------------- */

	function initScrollUI() {
		var doc = document.documentElement;
		var topbar = document.querySelector(".yss-topbar");
		var progress = null;
		if (topbar) {
			progress = document.createElement("div");
			progress.className = "yss-progress";
			progress.setAttribute("aria-hidden", "true");
			topbar.appendChild(progress);
		}

		var toTop = document.createElement("button");
		toTop.type = "button";
		toTop.className = "yss-totop";
		toTop.textContent = "↑";
		toTop.title = "回到顶部";
		toTop.setAttribute("aria-label", "回到顶部");
		toTop.addEventListener("click", function () {
			window.scrollTo({ top: 0, behavior: prefersReduced() ? "auto" : "smooth" });
		});
		document.body.appendChild(toTop);

		// 页面大标题：顶栏收起后接替它固定在顶部（大标题还没滚出去时不必重复显示）
		var titleEl = document.querySelector(".context h1.title") || document.querySelector("h1.title");
		var titleBar = null;
		if (titleEl) {
			titleBar = document.createElement("div");
			titleBar.className = "yss-titlebar";
			titleBar.setAttribute("aria-hidden", "true");
			var barInner = document.createElement("div");
			barInner.className = "yss-titlebar__inner";
			var barText = document.createElement("span");
			barText.className = "yss-titlebar__text";
			barText.textContent = titleEl.textContent.replace(/\s+/g, " ").trim();
			barInner.appendChild(barText);
			titleBar.appendChild(barInner);
			// 点标题栏回到顶部（与标题本身的行为一致）
			titleBar.addEventListener("click", function () {
				window.scrollTo({ top: 0, behavior: prefersReduced() ? "auto" : "smooth" });
			});
			document.body.appendChild(titleBar);
		}

		var lastY = window.scrollY || 0;
		var queued = false;
		var lastInput = 0;
		// 累计同方向滚动距离：手机上手指微抖会让方向反复切换，
		// 不做滞后的话顶栏会不停收起/滑出，看起来就是页面在抖。
		var downRun = 0;
		var upRun = 0;

		// 只有用户自己滚（滚轮/触摸/键盘）才收起顶栏：
		// 锚点跳转、抽屉展开引起的滚动位移（滚动锚定）不该把顶栏藏起来
		["wheel", "touchmove", "keydown"].forEach(function (type) {
			window.addEventListener(type, function () {
				lastInput = Date.now();
			}, { passive: true });
		});

		function paint() {
			queued = false;
			var y = window.scrollY || 0;
			var max = doc.scrollHeight - window.innerHeight;
			if (progress) {
				var ratio = max > 0 ? Math.min(1, Math.max(0, y / max)) : 0;
				progress.style.transform = "scaleX(" + ratio.toFixed(4) + ")";
				progress.classList.toggle("is-active", y > 48);
			}
			toTop.classList.toggle("is-visible", y > 700);
			if (topbar) {
				topbar.classList.toggle("is-scrolled", y > 8);
				var userDriven = Date.now() - lastInput < 1200;
				var delta = y - lastY;
				if (delta > 0) {
					downRun += delta;
					upRun = 0;
				} else if (delta < 0) {
					upRun -= delta;
					downRun = 0;
				}
				// 长页面才收起顶栏；短页面收起来没意义
				var longEnough = doc.scrollHeight > window.innerHeight * 2.5;
				if (!prefersReduced() && longEnough && userDriven && y > 280 && downRun > 90) {
					topbar.classList.add("is-hidden");
					downRun = 0;
				} else if (upRun > 40 || y <= 280) {
					topbar.classList.remove("is-hidden");
					upRun = 0;
				}
			}
			if (titleBar) {
				// 两个条件：顶栏已收起，且大标题本身已滚出视口
				var hidden = topbar ? topbar.classList.contains("is-hidden") : false;
				titleBar.classList.toggle("is-visible", hidden && titleEl.getBoundingClientRect().bottom < 0);
			}
			lastY = y;
		}

		function onScroll() {
			if (queued) {
				return;
			}
			queued = true;
			window.requestAnimationFrame(paint);
		}

		window.addEventListener("scroll", onScroll, { passive: true });
		window.addEventListener("resize", onScroll);
		// 点目录/锚点跳转后把顶栏放回来，免得标题被遮住
		window.addEventListener("hashchange", function () {
			if (topbar) {
				topbar.classList.remove("is-hidden");
			}
		});
		paint();
	}

	/* ----------------------------------------------------------
		8. 滚入视口时的渐显
		---------------------------------------------------------- */

	function initReveal() {
		// <head> 里的内联脚本已经在首帧前挂上了 yss-motion（并带 2.5 秒兜底解除），
		// 这里只负责逐个加 is-in：首屏的按文档顺序错开浮现，其余交给观察器。
		window.__yssMotionReady = true;
		if (prefersReduced() || !document.documentElement.classList.contains("yss-motion")) {
			return;
		}
		// 看门狗：万一样式与脚本版本不匹配（.is-in 不起作用）、或者过渡根本没跑，
		// 2.5 秒后直接解除动效类。宁可没有动画，也不能让正文停在透明状态。
		window.setTimeout(function () {
			var probe = document.querySelector(".context .is-in");
			if (!probe || parseFloat(window.getComputedStyle(probe).opacity) < 0.9) {
				document.documentElement.classList.remove("yss-motion");
			}
		}, 2500);
		var nodes = document.querySelectorAll(
			".context :is(h1, h2, h3, p, ul, ol, pre, .table, .admonition, .small-subtitle)"
		);
		// 表格/提示框内部的段落由容器整体带动画。
		// 注意 closest() 会匹配到元素自身，所以容器自己不能被跳过，
		// 否则表格永远拿不到 is-in、CSS 又排除了表格内部 → 整块表格永久透明。
		// 首屏（含已经滚过去的）元素立即按顺序浮现，避免用户盯着空白等
		var pending = [];
		var order = 0;
		each(nodes, function (el) {
			var container = el.closest(".table, .admonition");
			if (container && container !== el) {
				return;
			}
			if (el.getBoundingClientRect().top < window.innerHeight) {
				el.style.setProperty("--yss-reveal-delay", Math.min(order, 8) * 45 + "ms");
				el.classList.add("is-in");
				order++;
			} else {
				pending.push(el);
			}
		});
		if (!pending.length) {
			return;
		}
		if (!("IntersectionObserver" in window)) {
			each(pending, function (el) {
				el.classList.add("is-in");
			});
			return;
		}

		var observer = new IntersectionObserver(
			function (entries) {
				var step = 0;
				entries.forEach(function (entry) {
					if (!entry.isIntersecting) {
						return;
					}
					// 同一批里的元素依次错开，看起来像顺次浮现
					entry.target.style.setProperty("--yss-reveal-delay", Math.min(step, 4) * 45 + "ms");
					entry.target.classList.add("is-in");
					step++;
					observer.unobserve(entry.target);
				});
			},
			// 不收缩触发区域：收缩的话，刚好停在视口底部的元素（短页面滚到底时）
			// 可能永远碰不到触发区，就一直是不透明的了。
			{ rootMargin: "0px", threshold: 0.01 }
		);
		each(pending, function (el) {
			observer.observe(el);
		});
	}

	/* ----------------------------------------------------------
		9. 背景点阵（canvas）与鼠标涟漪

		点阵必须逐点位移才能做出涟漪，CSS 做不到，所以放在 canvas 上；
		色斑仍是 CSS 层（body::before）。没有波纹时只画一遍就停，
		不持续占帧；系统要求减少动态效果、或触屏设备上不生成涟漪。
		---------------------------------------------------------- */

	var DOT = {
		base: 20,          // 点阵间距（像素，屏幕大时自动变稀）
		radius: 1.35,      // 点半径
		life: 1.05,        // 波纹寿命（秒）
		speed: 480,        // 波纹扩散速度（像素/秒）
		sigma: 30,         // 波峰宽度
		reach: 560,        // 波纹影响半径（比原先大很多，波及更远）
		push: 4.5,         // 点的最大位移（比原先小，观感更克制）
		merge: 90,         // 与已有波纹中心近于此距离就不再叠一圈，而是并入
		minMove: 26        // 距上次触发起码要移动这么多（原地微动不触发）
	};

	function initDotField() {
		var canvas = document.createElement("canvas");
		canvas.className = "yss-dots";
		canvas.setAttribute("aria-hidden", "true");
		// 插到 body 最前面：与 body::before 同属 z-index:-1 层，但按 DOM 顺序在它之上
		document.body.insertBefore(canvas, document.body.firstChild);
		var ctx = canvas.getContext("2d");
		if (!ctx) {
			return;
		}

		var w = 0, h = 0, dpr = 1, spacing = DOT.base;
		var color = "rgba(0,0,0,0.1)";
		var bright = "rgba(0,0,0,0.34)";
		var TAU = 6.283185307179586;
		var ripples = [];
		var running = false;
		// 触屏（没有悬停指针）不开涟漪；减少动效时也不开
		var ripplesOn = !prefersReduced() && !(window.matchMedia && window.matchMedia("(hover: none)").matches);

		function readTheme() {
			var cs = getComputedStyle(document.documentElement);
			color = cs.getPropertyValue("--yss-grid").trim() || color;
			bright = cs.getPropertyValue("--yss-grid-bright").trim() || bright;
		}

		function draw(now) {
			ctx.clearRect(0, 0, w, h);
			var cols = Math.ceil(w / spacing) + 1;
			var rows = Math.ceil(h / spacing) + 1;
			for (var i = 0; i < cols; i++) {
				var px = i * spacing;
				for (var j = 0; j < rows; j++) {
					var py = j * spacing;
					var ox = 0, oy = 0, boost = 0;
					for (var k = 0; k < ripples.length; k++) {
						var rp = ripples[k];
						var age = (now - rp.t) / 1000;
						if (age < 0 || age > DOT.life) {
							continue;
						}
						var vx = px - rp.x;
						var vy = py - rp.y;
						var dist = Math.sqrt(vx * vx + vy * vy) || 1;
						if (dist > DOT.reach) {
							continue;
						}
						// 波峰随时间的环形位置；band 是点到波峰的高斯衰减
						var peak = dist - DOT.speed * age;
						var band = Math.exp(-(peak * peak) / (2 * DOT.sigma * DOT.sigma));
						var kk = band * (1 - dist / DOT.reach) * (1 - age / DOT.life) * (rp.amp || 1);
						if (kk > 0.002) {
							// 靠近波心时径向方向本身就是噪声（几个像素的偏差就能反转方向），
							// 所以让位移随距离平滑趋零，而不是让中心的点来回抰。
							var dirScale = dist < DOT.sigma ? dist / DOT.sigma : 1;
							ox += (vx / dist) * kk * DOT.push * dirScale;
							oy += (vy / dist) * kk * DOT.push * dirScale;
							boost += kk * dirScale;
						}
					}
					ctx.beginPath();
					ctx.arc(px + ox, py + oy, DOT.radius * (1 + boost * 0.7), 0, TAU);
					ctx.fillStyle = boost > 0.03 ? bright : color;
					ctx.fill();
				}
			}
		}

		function frame(now) {
			draw(now);
			var alive = [];
			for (var i = 0; i < ripples.length; i++) {
				if (now - ripples[i].t < DOT.life * 1000) {
					alive.push(ripples[i]);
				}
			}
			ripples = alive;
			if (ripples.length) {
				window.requestAnimationFrame(frame);
			} else {
				running = false;
				draw(0);
			}
		}

		function spawn(x, y, power) {
			var now = window.performance.now();
			var amp = power || 1;
			// 已经有一圈波就在附近：不再叠新的一圈，而是把那一圈的中心
			// 平滑拖过来并补一点能量。这样慢速移动只有一圈波在跟着手走，
			// 不会出现多个波心在指针附近互相对抽。
			for (var i = 0; i < ripples.length; i++) {
				var rp = ripples[i];
				var ddx = x - rp.x;
				var ddy = y - rp.y;
				if (ddx * ddx + ddy * ddy < DOT.merge * DOT.merge) {
					rp.x += ddx * 0.45;
					rp.y += ddy * 0.45;
					rp.amp = Math.min(1.6, (rp.amp || 1) + 0.1 * amp);
					return;
				}
			}
			if (ripples.length > 2) {
				ripples.shift();
			}
			ripples.push({ x: x, y: y, t: now, amp: amp });
			if (!running) {
				running = true;
				window.requestAnimationFrame(frame);
			}
		}

		function resize() {
			dpr = Math.min(window.devicePixelRatio || 1, 2);
			w = window.innerWidth;
			h = window.innerHeight;
			canvas.width = Math.floor(w * dpr);
			canvas.height = Math.floor(h * dpr);
			ctx.setTransform(dpr, 0, 0, dpr, 0, 0);
			// 屏幕很大时把点阵放稀一点，控制每帧绘制的点数
			spacing = w * h > 2200000 ? 24 : DOT.base;
			draw(0);
		}

		readTheme();
		resize();
		window.addEventListener("resize", function () {
			ripples = [];
			resize();
		});

		// 重新取色必须在下一帧：刚切完主题时 getComputedStyle 可能还是旧值
		function refresh() {
			window.requestAnimationFrame(function () {
				readTheme();
				draw(0);
			});
		}
		onThemeChange(refresh);
		if (window.MutationObserver) {
			new MutationObserver(refresh).observe(document.documentElement, {
				attributes: true,
				attributeFilter: ["data-theme"]
			});
		}
		if (window.matchMedia) {
			var scheme = window.matchMedia("(prefers-color-scheme: dark)");
			if (scheme.addEventListener) {
				scheme.addEventListener("change", refresh);
			} else if (scheme.addListener) {
				scheme.addListener(refresh);
			}
		}
		refresh();
		if (!ripplesOn) {
			return;
		}
		var last = { x: -1e4, y: -1e4, t: 0 };
		window.addEventListener("pointermove", function (e) {
			if (e.pointerType && e.pointerType !== "mouse") {
				return;
			}
			var now = window.performance.now();
			var dx = e.clientX - last.x;
			var dy = e.clientY - last.y;
			// 必须真的移动了一定距离才触发：原地微动（手在鼠标上轻微飘）不该搅动点阵
			if (Math.sqrt(dx * dx + dy * dy) < DOT.minMove || now - last.t < 60) {
				return;
			}
			last.x = e.clientX;
			last.y = e.clientY;
			last.t = now;
			spawn(e.clientX, e.clientY);
		}, { passive: true });

		// 按一下给一圈更足的波（amp 1.5），点击就有明确反馈；
		// 顺手把 last 同步过去，免得松手后缓慢拖出一个多余的波纹。
		window.addEventListener("pointerdown", function (e) {
			if (e.pointerType && e.pointerType !== "mouse") {
				return;
			}
			last.x = e.clientX;
			last.y = e.clientY;
			last.t = window.performance.now();
			spawn(e.clientX, e.clientY, 1.5);
		}, { passive: true });
	}

	function ready() {
		initTheme();
		initMenu();
		// 先补目录：它会给缺 id 的分组标题分配锚点，后面的标题锚点、滚动高亮都要用
		initTocExtra();
		initHeadingAnchors();
		initScrollSpy();
		initCodeCopy();
		initCurrentNav();
		initScrollUI();
		initReveal();
		initDotField();
	}

	if (document.readyState === "loading") {
		document.addEventListener("DOMContentLoaded", ready);
	} else {
		ready();
	}
})();
