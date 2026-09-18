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
	8. 背景点阵与鼠标涟漪（canvas 绘制，色晕只是 CSS 上的一层固定淡底）；
	9. 右上角的背景设置区（上传图片 / 清除 / 透明度 / 背景模糊，图片只存在本机浏览器里）
	   以及面板折射：面板自己的 backdrop-filter 引用 SVG 位移滤镜，把背后的照片真的掰弯。
*/

(function () {
	"use strict";

	var THEME_KEY = "yss-doc-theme";
	// 主题变化时要跟着更新的东西（目前只有背景点阵的颜色）
	var themeListeners = [];
	// 入场动效（元素用 translateY 浮现）期间，玻璃面板的位置在变，
	// 折射带得跟着重算。initDotField() 会把这个钩子接上。
	var glassSettle = null;
	// 外部动了"与玻璃有关的东西"（如给面板挂上折射滤镜）后，重画一帧点阵
	var glassRedraw = null;

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
		if (order && glassSettle) {
			glassSettle();
		}
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
				if (step && glassSettle) {
					glassSettle();
				}
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
		9. 背景点阵（canvas）、玻璃边缘折射与鼠标涟漪

		点阵必须逐点位移才能做出涟漪和折射，CSS 做不到，所以放在 canvas 上；
		色斑仍是 CSS 层（body::before）。没有波纹时只画一遍就停，不持续占帧；
		系统要求减少动态效果、或触屏设备上不生成涟漪（折射是静态的，照旧）。
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

	/* 玻璃面板的边缘折射（对应 Visindigo::Widgets::LiquidGlassEffect 的 distortImage）。
		selector 必须与 CSS 里应用玻璃的那一串保持一致 ——
		两边不一致的话，折射带会画在不是玻璃的地方（或者漏掉某块玻璃）。
		lens 对应他们的 liquidDistortRadius，dispersion 对应 liquidDistortDispersion。 */
	var GLASS = {
		lens: 20,
		minLens: 5,
		dispersion: 0,
		dispMin: 0.6,
		dispMax: 8,
		// 几何位移由谁来扭：true = 点阵自己扭（第 9 节）；
		// false = 交给面板自己的 backdrop-filter（第 11 节，连照片一起扭），
		// 此时第 9 节只剩色散，否则会被扭两遍。
		warp: true,
		selector: ".table,pre,.admonition,.toc,.context ul,.context ol"
	};

	var glassList = null;   // 玻璃面板节点，只查一次

	// 符合条件的玻璃面板（嵌在另一块玻璃里的不算 —— 会叠成两层折射）
	function glassNodes() {
		if (glassList) {
			return glassList;
		}
		glassList = [];
		var all = document.querySelectorAll(GLASS.selector);
		for (var i = 0; i < all.length; i++) {
			if (all[i].parentElement && all[i].parentElement.closest(GLASS.selector)) {
				continue;
			}
			glassList.push(all[i]);
		}
		return glassList;
	}

	/* 玻璃边缘的鼠标边缘光（对应他们的 EffectType::RimLight / rimLightGlow）。
		range 对应 rimLightRange，activate 对应 rimLightActivationDistance（两个都从 CSS 读，
		改 CSS 变量即可；activate 为 0 就等于关掉这个效果）。 */
	var RIM = {
		range: 80,
		activate: 40
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
		var bgColor = "#ffffff";   // 页面底色，用来拼色散的单通道色
		// 色散用的三个单通道色（见 readTheme）
		var chanR = "rgba(0,0,0,0.04)", chanG = chanR, chanB = chanR;
		var TAU = 6.283185307179586;
		var ripples = [];
		var running = false;
		// 折射相关
		var radiusPx = 10;      // 面板圆角（取自 CSS 的 --yss-radius，裁剪折射带要用）
		var baseLayer = null;   // 静态点阵的离屏缓存：没有波纹时只 blit，不重画上万个点
		var drawQueued = false;
		var settleUntil = 0;
		var settleRunning = false;
		// 触屏（没有悬停指针）不开涟漪；减少动效时也不开
		var ripplesOn = !prefersReduced() && !(window.matchMedia && window.matchMedia("(hover: none)").matches);

		function readTheme() {
			var cs = getComputedStyle(document.documentElement);
			color = cs.getPropertyValue("--yss-grid").trim() || color;
			bright = cs.getPropertyValue("--yss-grid-bright").trim() || bright;
			bgColor = cs.getPropertyValue("--yss-bg").trim() || bgColor;
			// 面板圆角，与 CSS 的 --yss-radius 一致
			var rv = parseFloat(cs.getPropertyValue("--yss-radius"));
			if (isFinite(rv) && rv > 0) {
				radiusPx = rv;
			}
			// 折射带宽度与色散量也从 CSS 读，方便调（见 :root 里的 --yss-glass-lens / -dispersion）
			var lv = parseFloat(cs.getPropertyValue("--yss-glass-lens"));
			if (isFinite(lv) && lv > 0) {
				GLASS.lens = lv;
			}
			var lrv = parseFloat(cs.getPropertyValue("--yss-glass-lens-ratio"));
			if (isFinite(lrv) && lrv >= 0) {
				LENS.ratio = lrv;
			}
			var dv = parseFloat(cs.getPropertyValue("--yss-glass-dispersion"));
			if (isFinite(dv) && dv > 0) {
				GLASS.dispersion = dv;
			}
			// 边缘光的两个距离，同样以 CSS 为准
			var ra = parseFloat(cs.getPropertyValue("--yss-glass-rim-activate"));
			if (isFinite(ra) && ra >= 0) {
				RIM.activate = ra;
			}
			var rr = parseFloat(cs.getPropertyValue("--yss-glass-rim-range"));
			if (isFinite(rr) && rr > 0) {
				RIM.range = rr;
			}
			// 色散用的单通道色：把**点色的那个通道**叠到**页面底色的另两个通道**上。
			// 这样用普通 alpha 合成画出来，就等于「只让这一个通道被点影响」——
			// 亮底上留下青/品红/黄，暗底上留下红/绿/蓝，两种主题都是对的色散。
			// （之前用纯通道色 + lighter 只在深色下对：浅色下三颗点都是同一个灰，就成了"重复的三个点"。）
			var pc = parseColor(color);
			var bc = parseColor(bgColor);
			if (pc && bc) {
				chanR = "rgba(" + pc[0] + "," + bc[1] + "," + bc[2] + "," + pc[3] + ")";
				chanG = "rgba(" + bc[0] + "," + pc[1] + "," + bc[2] + "," + pc[3] + ")";
				chanB = "rgba(" + bc[0] + "," + bc[1] + "," + pc[2] + "," + pc[3] + ")";
			}
		}

		/* ---- 玻璃边缘折射（对应 LiquidGlassEffect::distortImage）----
			他们的映射是「输出位置 → 采样源位置」：带内距边缘 x 处，采样源图上距边缘
				nearMap(x) = R/2 + (1 + sin(π/2 · x/R − π/2)) · R/2
			处的像素（x ∈ [0, R]，值域 R/2 .. R）：贴边处从 R/2 采、带的内边界处就地采 ——
			于是带内约 2 倍放大（点距被拉开一倍），而源图上最外那 R/2 圈不会被任何输出位置采到。
			canvas 上画的是稀疏点阵，没法逐输出像素重采样，所以对每个**源点**反解它该画在哪：
				s = R/2 + u · R/2   →   u = (s − R/2) / (R/2)
				x = (2R/π) · (π/2 + asin(u − 1))
			（采样不到的源点原地不动，见 refractPoint 的注释。）
			色散：他们的实现是把 channelScale 乘在**采样偏移**上（nearMap 的第三项），
			那是给"输出重采样"用的 —— 换成"源点重投影"之后，那个写法会变成
			"越靠带的内边界分离越大、贴边处反而是 0"，与真实玻璃正好相反。
			所以这里按真实玻璃的观感来：分离量以边缘为最大，向内线性衰减到 0（见 refractSpread）。 */

		/* 源点（距边缘 s）折射后该落在距边缘多远。
			【为什么没有直接用他们的 nearMap 逆】
			他们的映射把源 [R/2, R] 映满输出 [0, R]（平均 2 倍放大），代价是最外 R/2 那圈源点
			**没有任何输出位置** —— 在连续图像上那是"被放大后的内容盖住"，看不出问题；
			但在 20px 间隔的点阵上，那是一条 R/2 宽的死带（R=48 时 24px > 点距），
			必然至少压住一整列点，看起来就是"这一列压根没反应"。
			所以改用一条覆盖整个 [0, R] 的单调曲线：边缘拉开、内侧按比例压一点来守恒
			（平均放大率必须为 1，不可能只放大边缘），于是每个源点都有位移、越靠边越大。 */
		function refractPos(s, R) {
			var t = s / R;
			if (t <= 0 || t >= 1) {
				return null;
			}
			// out'(0) = 1.6（边缘点距拉开 60%）、out'(1) = 0.4（内侧压到 40%）
			return R * (1.6 * t - 0.6 * t * t);
		}

		// 沿某个轴（a0..a1）求折射后的位置；null = 这个源点不在被采样的区间里
		function refractAxis(p, a0, a1, R) {
			// 面板外的点（循环会多取一格网格）不参与折射，也不能被当成带内丢掉
			if (p <= a0 || p >= a1) {
				return p;
			}
			if (p - a0 < R) {
				var q = refractPos(p - a0, R);
				return q === null ? null : a0 + q;
			}
			if (a1 - p < R) {
				var q2 = refractPos(a1 - p, R);
				return q2 === null ? null : a1 - q2;
			}
			return p;
		}

		/* 一个源点在某个通道下的落脚点。ch = 通道系数，1 就是主映射（不带色散）。
			注意这里**不再把 null 当成"删掉这个点"**：他们的映射是给连续图像用的 ——
			最外 R/2 那圈源内容采不到，被放大后的内容盖住，图像上根本看不出空。
			我们的背景是 20px 间隔的离散点，同样的映射就会变成"边缘挖掉一两个点"，
			而哪些点被挖是网格相位决定的（每个面板、每条边都不同），
			于是四条边看起来各行其是、某一侧"没被扭曲"。
			所以：采不到的源点原地不动 —— 点阵不断、四边一致。 */
		function refractPoint(dx, dy, panel, ch) {
			var px = dx, py = dy;
			var R = panel.R;
			// GLASS.warp 关掉时几何位移由面板自己的 backdrop-filter 负责（第 11 节），
			// 这里就只剩色散：三颗点各偏一点，之后一起被滤镜掰弯，
			// 于是贴边的彩边还被顺带放大 —— 真玻璃就是这样。
			if (GLASS.warp) {
				if (panel.hx) {
					var q = refractAxis(dx, panel.x0, panel.x1, R);
					px = q === null ? dx : q;
				}
				if (panel.hy) {
					var q2 = refractAxis(dy, panel.y0, panel.y1, R);
					py = q2 === null ? dy : q2;
				}
			}
			// 色散两个轴都要错开（他们的三通道映射在 x、y 上各查一次表）
			px = refractSpread(px, panel.x0, panel.x1, R, ch);
			py = refractSpread(py, panel.y0, panel.y1, R, ch);
			return [px, py];
		}

		// 色散偏移：贴边处最大，到折射带的内边界线性衰减到 0
		function refractSpread(v, a0, a1, R, ch) {
			if (ch === 1) {
				return v;
			}
			var de = Math.min(v - a0, a1 - v);
			var f = 1 - Math.max(0, Math.min(1, de / R));
			return v + (1 - ch) * R * 0.5 * f;
		}

		// 解析 CSS 颜色（#rgb / #rrggbb / rgb[a](...)）成 [r, g, b, a]
		function parseColor(c) {
			var s = String(c).trim();
			var h = s.match(/^#([0-9a-f]{3}|[0-9a-f]{6})$/i);
			if (h) {
				var v = h[1];
				if (v.length === 3) {
					v = v[0] + v[0] + v[1] + v[1] + v[2] + v[2];
				}
				return [parseInt(v.substr(0, 2), 16), parseInt(v.substr(2, 2), 16), parseInt(v.substr(4, 2), 16), 1];
			}
			var m = s.match(/[\d.]+/g);
			if (!m || m.length < 4) {
				return null;
			}
			return [parseFloat(m[0]), parseFloat(m[1]), parseFloat(m[2]), parseFloat(m[3])];
		}

		// 当前可见的玻璃面板矩形（视口坐标，与 canvas 同一套坐标）
		function glassRects() {
			var nodes = glassNodes();
			var list = [];
			for (var i = 0; i < nodes.length; i++) {
				var r = nodes[i].getBoundingClientRect();
				if (r.width < 8 || r.height < 8 || r.bottom < 0 || r.top > h || r.right < 0 || r.left > w) {
					continue;
				}
				// 他们也是把半径夹到短边的一半，免得带比面板还宽
				var R = lensRadius(r.width, r.height);
				if (R < GLASS.minLens) {
					continue;
				}
				// 带刚好铺满半个面板（R = 短边/2）时两条带在中间相接 —— 他们 C++ 里
				// rightStart == R 时也照样成立，所以这里允许 2R 顶到边长。
				list.push({
					x0: r.left, y0: r.top, x1: r.right, y1: r.bottom, R: R,
					hx: r.width + 2 >= 2 * R,
					hy: r.height + 2 >= 2 * R
				});
			}
			return list;
		}

		/* ---- 鼠标边缘光（对应 LiquidGlassEffect 的 EffectType::RimLight）----
			他们的规则：边界上离鼠标最近的那个点最亮，其余边界点按
				weight = (1 - (d - dmin) / range)²
			衰减（d 是边界点到鼠标的距离，dmin 是所有边界点里最近的那个）；整个面板还要乘上
			「激活强度」（rimLightIntensityAt）：鼠标在面板内为 1，到面板的距离超过
			activationDistance 归零，中间按 (1 - d / 激活距离)² 过渡。
			所以这里只要给面板写四个边界权重和一个边界上的位置，
			形状与颜色在 CSS 的 --yss-rim-bg 里 —— 不用伪元素，理由与折射那里一样。 */

		var rimRects = null;   // 可见面板的视口矩形缓存，只在滚动/尺寸变化后重建

		// 面板在视口内的矩形缓存。每帧现读 getBoundingClientRect 会强迫布局（旁边刚写过样式），
		// 而滚动时面板的视口位置会变，所以在滚动/缩放/切主题后失效即可。
		function buildRimRects() {
			rimRects = [];
			var nodes = glassNodes();
			for (var i = 0; i < nodes.length; i++) {
				var r = nodes[i].getBoundingClientRect();
				if (r.width < 8 || r.height < 8) {
					continue;
				}
				if (r.bottom < -RIM.activate || r.top > h + RIM.activate || r.right < -RIM.activate || r.left > w + RIM.activate) {
					continue;
				}
				rimRects.push({ el: nodes[i], x0: r.left, y0: r.top, x1: r.right, y1: r.bottom });
			}
		}

		// 与 rimLightGlow 的权重一致；d 为某条边的最近点到鼠标的距离，dmin 为四条边里最小的
		function rimWeight(d, dmin) {
			var w = 1 - (d - dmin) / RIM.range;
			return w > 0 ? w * w : 0;
		}

		// 面板自己的状态没变就别写样式：鼠标在面板内部移动时，多数面板的四个数都是 0
		function setRim(q, t, b, l, r, rx, ry) {
			t = Math.round(t * 1000) / 1000;
			b = Math.round(b * 1000) / 1000;
			l = Math.round(l * 1000) / 1000;
			r = Math.round(r * 1000) / 1000;
			rx = Math.round(rx * 10) / 10;
			ry = Math.round(ry * 10) / 10;
			if (q.t === t && q.b === b && q.l === l && q.r === r && q.rx === rx && q.ry === ry) {
				return;
			}
			q.t = t;
			q.b = b;
			q.l = l;
			q.r = r;
			q.rx = rx;
			q.ry = ry;
			var st = q.el.style;
			st.setProperty("--rl-t", t);
			st.setProperty("--rl-b", b);
			st.setProperty("--rl-l", l);
			st.setProperty("--rl-r", r);
			st.setProperty("--rl-x", rx + "px");
			st.setProperty("--rl-y", ry + "px");
		}

		function clearRim() {
			if (rimRects) {
				for (var i = 0; i < rimRects.length; i++) {
					setRim(rimRects[i], 0, 0, 0, 0, 0, 0);
				}
			}
		}

		function updateRim(x, y) {
			if (!rimRects) {
				buildRimRects();
			}
			for (var i = 0; i < rimRects.length; i++) {
				var q = rimRects[i];
				// 激活强度：鼠标在面板内为 0 距离（强度 1），离开边界 activate 距离后熄灭
				var ox = q.x0 > x ? q.x0 - x : (x > q.x1 ? x - q.x1 : 0);
				var oy = q.y0 > y ? q.y0 - y : (y > q.y1 ? y - q.y1 : 0);
				var dist = Math.sqrt(ox * ox + oy * oy);
				if (dist >= RIM.activate) {
					setRim(q, 0, 0, 0, 0, 0, 0);
					continue;
				}
				var act = dist <= 0 ? 1 : (1 - dist / RIM.activate) * (1 - dist / RIM.activate);
				// 四条边各自的「离鼠标最近的点」：横向边只夹 x，竖向边只夹 y
				var cx = x < q.x0 ? q.x0 : (x > q.x1 ? q.x1 : x);
				var cy = y < q.y0 ? q.y0 : (y > q.y1 ? q.y1 : y);
				var ddx = x - cx;
				var ddy = y - cy;
				var dt = Math.sqrt(ddx * ddx + (y - q.y0) * (y - q.y0));
				var db = Math.sqrt(ddx * ddx + (y - q.y1) * (y - q.y1));
				var dl = Math.sqrt((x - q.x0) * (x - q.x0) + ddy * ddy);
				var dr = Math.sqrt((x - q.x1) * (x - q.x1) + ddy * ddy);
				var dmin = Math.min(Math.min(dt, db), Math.min(dl, dr));
				setRim(q,
					act * rimWeight(dt, dmin),
					act * rimWeight(db, dmin),
					act * rimWeight(dl, dmin),
					act * rimWeight(dr, dmin),
					cx - q.x0,
					cy - q.y0);
			}
		}

		function drawDots(g, x0, y0, x1, y1, now, panel) {
			var i0 = Math.max(0, Math.floor(x0 / spacing));
			var i1 = Math.min(Math.ceil(w / spacing), Math.ceil(x1 / spacing));
			var j0 = Math.max(0, Math.floor(y0 / spacing));
			var j1 = Math.min(Math.ceil(h / spacing), Math.ceil(y1 / spacing));
			for (var i = i0; i <= i1; i++) {
				var dx = i * spacing;
				for (var j = j0; j <= j1; j++) {
					var dy = j * spacing;
					var px = dx, py = dy;
					var chan = null;
					// 滤镜路径生效时（lensActive）几何与色散都交给面板的 backdrop-filter，
					// 点阵画成直的、跟背板一起被掰弯即可 —— 这里就不用逐点算位移了。
					// 一帧要过几千个点，这条分支能省下每帧两次函数调用。
					if (panel && !lensActive) {
						var mid = refractPoint(dx, dy, panel, 1);
						px = mid[0];
						py = mid[1];
						// 色散：三个通道位置错开得够多才拆开画，否则还是一个点（免得整体变淡）
						// 滤镜路径生效时色散由滤镜做（三通道三张表），别再自己拆一遍
						if (GLASS.dispersion > 0 && !lensActive) {
							var lo = refractPoint(dx, dy, panel, 1 - GLASS.dispersion);
							var hi = refractPoint(dx, dy, panel, 1 + GLASS.dispersion);
							var spread = Math.abs(lo[0] - hi[0]) + Math.abs(lo[1] - hi[1]);
							// 太小 = 看不出来；太大 = 某个通道已经落到采样区外（那位是"原地不动"的），
							// 两种情况都照常画一个点，别硬拆
							if (spread >= GLASS.dispMin && spread <= GLASS.dispMax) {
								chan = [lo, mid, hi];
							}
						}
					}
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
					if (chan) {
						// 色散：三颗点各只带一个通道（另两个通道用的是页面底色），
						// 所以普通合成就行 —— 错开的地方显彩边，重叠的地方还是原来的灰
						for (var c = 0; c < 3; c++) {
							g.beginPath();
							g.arc(chan[c][0] + ox, chan[c][1] + oy, DOT.radius * (1 + boost * 0.7), 0, TAU);
							g.fillStyle = c === 0 ? chanR : (c === 1 ? chanG : chanB);
							g.fill();
						}
					} else {
						g.beginPath();
						g.arc(px + ox, py + oy, DOT.radius * (1 + boost * 0.7), 0, TAU);
						g.fillStyle = boost > 0.03 ? bright : color;
						g.fill();
					}
				}
			}
		}

		// 静态点阵的离屏缓存。没有波纹时每帧只需要 blit 一次，
		// 不必把上万个点重画一遍（滚动时每一帧都要重画折射带）。
		function buildBase() {
			baseLayer = null;
			if (!w || !h || !canvas.width) {
				return;
			}
			var off = document.createElement("canvas");
			off.width = canvas.width;
			off.height = canvas.height;
			var g = off.getContext("2d");
			if (!g) {
				return;
			}
			g.setTransform(dpr, 0, 0, dpr, 0, 0);
			drawDots(g, 0, 0, w, h, 0, null);
			baseLayer = off;
		}

		function draw(now) {
			var rects = glassRects();
			ctx.clearRect(0, 0, w, h);
			if (ripples.length || !baseLayer) {
				drawDots(ctx, 0, 0, w, h, now, null);
			} else {
				ctx.drawImage(baseLayer, 0, 0, w, h);
			}
			for (var i = 0; i < rects.length; i++) {
				var p = rects[i];
				// 先裁剪到面板的圆角矩形再擦：圆角外面露的是页面背景，
				// 直接按矩形擦会在那里碰掉一两颗点
				ctx.save();
				ctx.beginPath();
				if (ctx.roundRect) {
					ctx.roundRect(p.x0, p.y0, p.x1 - p.x0, p.y1 - p.y0, radiusPx);
				} else {
					ctx.rect(p.x0, p.y0, p.x1 - p.x0, p.y1 - p.y0);
				}
				ctx.clip();
				ctx.clearRect(p.x0, p.y0, p.x1 - p.x0, p.y1 - p.y0);
				drawDots(ctx, p.x0, p.y0, p.x1, p.y1, now, p);
				ctx.restore();
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
			/* 坐标空间必须与画布在屏幕上**实际占的尺寸严格 1:1**，否则整张图会被缩放。
				画布是 position:fixed + inset:0，包含块是初始包含块，它的宽度**不含垂直滚动条**
				（clientWidth 1686 vs innerWidth 1701，画布被水平压扁 0.86%）——
				缩放偏差随 x 线性增长：左边缘几乎看不出来，越靠右偏得越多，到面板右边缘
				已经是十几像素，折射带整体落在边缘左侧，最外那列点落在带外一动不动。
				这就是"左、上、下都像折射过，只有右侧不对"的来源。
				所以从文档元素的客户区取 w/h，并把画布的 CSS 尺寸按同一个 w/h 显式写死
				（用取舍后的整数像素反推，w*dpr 带小数时也不留残余缩放）。 */
			w = document.documentElement.clientWidth || window.innerWidth;
			h = document.documentElement.clientHeight || window.innerHeight;
			canvas.width = Math.round(w * dpr);
			canvas.height = Math.round(h * dpr);
			canvas.style.width = canvas.width / dpr + "px";
			canvas.style.height = canvas.height / dpr + "px";
			ctx.setTransform(dpr, 0, 0, dpr, 0, 0);
			// 屏幕很大时把点阵放稀一点，控制每帧绘制的点数
			spacing = w * h > 2200000 ? 24 : DOT.base;
			rimRects = null;
			buildBase();
			draw(0);
		}

		readTheme();
		resize();
		window.addEventListener("resize", function () {
			ripples = [];
			resize();
		});

		// 面板跟着页面滚，而点阵是画在视口坐标上的，所以滚动时折射带要重算。
		// 用 rAF 合并成每帧最多一次；涟漪在跑时它本来就在逐帧重画，直接跳过。
		function scheduleDraw() {
			if (drawQueued || running) {
				return;
			}
			drawQueued = true;
			window.requestAnimationFrame(function () {
				drawQueued = false;
				// 面板在屏幕上移动了，折射带和边缘光都得重新定位
				rimRects = null;
				if (rimLast.x > -1e3) {
					updateRim(rimLast.x, rimLast.y);
				}
				draw(0);
			});
		}
		// 入场动效那 0.6 秒里面板还在往上浮，静止的折射带会落在错的位置。
		// 多处同时触发时共用同一个循环，不要一人开一个 rAF。
		glassSettle = function () {
			settleUntil = window.performance.now() + 800;
			if (settleRunning) {
				return;
			}
			settleRunning = true;
			(function tick() {
				if (!running) {
					draw(0);
				}
				if (window.performance.now() < settleUntil) {
					window.requestAnimationFrame(tick);
				} else {
					settleRunning = false;
				}
			})();
		};
		// 给外面用的重画入口（第 11 节挂上折射滤镜之后点阵不该再自己扭了，要重画一帧）
		glassRedraw = scheduleDraw;
		if (glassNodes().length) {
			window.addEventListener("scroll", scheduleDraw, { passive: true });
		}

		// 重新取色必须在下一帧：刚切完主题时 getComputedStyle 可能还是旧值
		function refresh() {
			window.requestAnimationFrame(function () {
				readTheme();
				rimRects = null;
				buildBase();
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

		/* 鼠标边缘光。他们那套是挂在控件、窗口、QWindow 三级上的事件过滤器；
			网页里一个 passive 的 pointermove 就够，且用 rAF 合并成每帧最多算一次。
			触屏没有「靠近」，直接不接。 */
		var rimLast = { x: -1e4, y: -1e4 };
		if (RIM.activate > 0 && !(window.matchMedia && window.matchMedia("(hover: none)").matches)) {
			var rimQueued = false;
			window.addEventListener("pointermove", function (e) {
				if (e.pointerType && e.pointerType !== "mouse") {
					return;
				}
				rimLast.x = e.clientX;
				rimLast.y = e.clientY;
				if (rimQueued) {
					return;
				}
				rimQueued = true;
				window.requestAnimationFrame(function () {
					rimQueued = false;
					updateRim(rimLast.x, rimLast.y);
				});
			}, { passive: true });
			// 鼠标出了窗口就把所有边缘光熄掉，不然会留一条亮线在那
			function rimOff() {
				rimLast.x = -1e4;
				rimLast.y = -1e4;
				clearRim();
			}
			document.addEventListener("pointerleave", rimOff);
			window.addEventListener("blur", rimOff);
		}
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

	/* ----------------------------------------------------------
		10. 背景设置（右上角设置区：上传 / 清除 / 透明度 / 模糊度）

		上传的图片只存在**本机浏览器**里，不上服务器（这个文档站没有后端）。
		优先 IndexedDB：它能直接存二进制，不像 localStorage 既要 base64（体积再涨三分之一）
		又只有 5MB；IndexedDB 用不了（无痕模式、某些浏览器下的 file://）才退回
		localStorage 存 data URL —— 所以兜底那一路正好卡在 5MB 以内。

		存之前先把长边缩到 BG.maxSide 再重编码：文档背景不需要原始分辨率，
		缩完一份几百 KB，上面两条路都放得下，每次打开的解码也快得多（一张 5MB 的
		照片光解码就够用户对着空白发一会儿呆了）。

		【为什么收成一个设置面板，而不是顶栏上四个控件】上传、清除、透明度、模糊度
		是同一件事的四个面，摊在顶栏上就是四个按钮，收进一个弹出面板才叫"设置"。
		面板挂在 body 上而不是顶栏里：顶栏自己带 backdrop-filter，那是一个 backdrop root，
		挂在里面的面板再怎么 blur 也只能糊到顶栏那一层背板，出了顶栏范围就什么都没有。
		所以位置在每次打开时按按钮的矩形算，见 placePop()。

		渐进增强：按钮写在顶栏 HTML 里，脚本没跑起来（或浏览器太老、连 Promise 都没有）
		时它就是个没反应的胶囊，页面上不会多出别的东西 —— 面板和文件选择框都是脚本建的。
		---------------------------------------------------------- */

	var BG = {
		btnId: "yss-bg-toggle",
		popId: "yss-bgset",
		db: "yss-doc-bg",
		store: "files",
		key: "image",
		lsKey: "yss-doc-bg-image",   // 兜底那一路的键；存进 IndexedDB 成功时会清掉它
		optKey: "yss-doc-bg-opt",    // { v, alpha, blur } 一起存成一条 JSON
		// 改了下面两个默认值就把这个数字 +1：版本号对不上的旧数据不再沿用（否则
		// 存过一次滑块的页面会永远停在旧默认上），直接用新默认并回写一条带新版本号的。
		optVer: 2,
		maxFile: 32 * 1024 * 1024,   // 再大就不读了，读进内存也存不下
		maxSide: 2560,               // 存储用的长边上限
		quality: 0.9,
		defAlpha: 25,                // 透明度默认值（%）：盖在图片上的那层薄纱
		defBlur: 3,                  // 背景模糊默认值（px）
		maxBlur: 80                  // 模糊滑块上限（px）
	};

	function idbOpen() {
		return new Promise(function (resolve, reject) {
			if (!window.indexedDB) {
				reject(new Error("no-indexeddb"));
				return;
			}
			var req = window.indexedDB.open(BG.db, 1);
			req.onupgradeneeded = function () {
				req.result.createObjectStore(BG.store);
			};
			req.onsuccess = function () {
				resolve(req.result);
			};
			req.onerror = function () {
				reject(req.error || new Error("idb-open"));
			};
		});
	}

	// 跑一个事务，成功时解析成那次请求的结果
	function idbRun(mode, job) {
		return idbOpen().then(function (db) {
			return new Promise(function (resolve, reject) {
				var tx = db.transaction(BG.store, mode);
				var req = job(tx.objectStore(BG.store));
				tx.oncomplete = function () {
					resolve(req ? req.result : undefined);
				};
				tx.onerror = tx.onabort = function () {
					reject(tx.error || new Error("idb-tx"));
				};
			});
		});
	}

	function blobToDataURL(blob) {
		return new Promise(function (resolve, reject) {
			var fr = new FileReader();
			fr.onload = function () {
				resolve(String(fr.result));
			};
			fr.onerror = function () {
				reject(fr.error || new Error("read"));
			};
			fr.readAsDataURL(blob);
		});
	}

	// 返回能直接塞进 url() 的地址：IndexedDB 里取出的是 blob:，兜底那路读出来是 data:
	function bgLoadURL() {
		return idbRun("readonly", function (s) {
			return s.get(BG.key);
		}).then(function (blob) {
			return blob instanceof Blob ? URL.createObjectURL(blob) : null;
		}, function () {
			return null;
		}).then(function (url) {
			if (url) {
				return url;
			}
			try {
				return localStorage.getItem(BG.lsKey);
			} catch (e) {
				return null;
			}
		});
	}

	// 两条路都写不进去就返回 false（图太大，浏览器不给存）
	function bgSave(blob) {
		return idbRun("readwrite", function (s) {
			return s.put(blob, BG.key);
		}).then(function () {
			try {
				localStorage.removeItem(BG.lsKey);
			} catch (e) {
				/* 清不掉也没关系：下次读的是 IndexedDB 里那份 */
			}
			return true;
		}, function () {
			return blobToDataURL(blob).then(function (url) {
				try {
					localStorage.setItem(BG.lsKey, url);
					return true;
				} catch (e) {
					return false;
				}
			});
		});
	}

	// 两处都清；清不掉就留着（界面已经清了，不值得为这个弹报错，见 clear）
	function bgDrop() {
		idbRun("readwrite", function (s) {
			return s.delete(BG.key);
		}).then(null, function () {
			return null;
		});
		try {
			localStorage.removeItem(BG.lsKey);
		} catch (e) {
			/* 同 bgSave */
		}
	}

	// WebP 优先（同观感体积小一半）；编码格式不被支持时浏览器会退回 PNG，那就再试 JPEG
	function bgEncode(cv) {
		return new Promise(function (resolve, reject) {
			if (!cv.toBlob) {
				reject(new Error("no-toblob"));
				return;
			}
			function enc(type) {
				cv.toBlob(function (blob) {
					if (blob && blob.type === type) {
						resolve(blob);
					} else if (type === "image/webp") {
						enc("image/jpeg");
					} else {
						reject(new Error("encode"));
					}
				}, type, BG.quality);
			}
			enc("image/webp");
		});
	}

	// 长边缩到 maxSide 再重编码，返回可以直接存起来的 Blob
	function bgShrink(file) {
		return new Promise(function (resolve, reject) {
			var url = URL.createObjectURL(file);
			var img = new Image();
			function done(err, blob) {
				URL.revokeObjectURL(url);
				if (err) {
					reject(err);
				} else {
					resolve(blob);
				}
			}
			img.onload = function () {
				var scale = Math.min(1, BG.maxSide / Math.max(img.naturalWidth, img.naturalHeight));
				var cv = document.createElement("canvas");
				cv.width = Math.max(1, Math.round(img.naturalWidth * scale));
				cv.height = Math.max(1, Math.round(img.naturalHeight * scale));
				var g = cv.getContext("2d");
				if (!g) {
					done(new Error("no-2d"));
					return;
				}
				g.drawImage(img, 0, 0, cv.width, cv.height);
				bgEncode(cv).then(function (blob) {
					done(null, blob);
				}, function (err) {
					done(err);
				});
			};
			img.onerror = function () {
				done(new Error("decode"));
			};
			img.src = url;
		});
	}

	function initUserBackground() {
		var button = document.getElementById(BG.btnId);
		if (!button || !window.Promise) {
			return;
		}

		// 文件选择框由脚本建：脚本没跑起来时页面上不该多出一个没用的控件。
		// 用移出视口而不是 display:none —— Safari 对看不见的 input 偶尔不肯弹选择器。
		var input = document.createElement("input");
		input.type = "file";
		input.accept = "image/*";
		input.setAttribute("tabindex", "-1");
		input.setAttribute("aria-hidden", "true");
		input.style.cssText = "position:fixed;left:-9999px;top:0;width:1px;height:1px;opacity:0;";
		document.body.appendChild(input);

		/* ---- 设置面板 ---- */

		var pop = document.createElement("div");
		pop.className = "yss-bgset";
		pop.id = BG.popId;
		pop.hidden = true;
		pop.setAttribute("role", "dialog");
		pop.setAttribute("aria-label", "背景设置");
		pop.innerHTML =
			'<div class="yss-bgset__row">' +
			'<button type="button" class="yss-roundbtn" data-yss="pick" title="上传背景图片" aria-label="上传背景图片">＋</button>' +
			'<button type="button" class="yss-roundbtn" data-yss="clear" title="清除背景图片" aria-label="清除背景图片">✕</button>' +
			'</div>' +
			'<label class="yss-bgset__field"><span>透明度</span>' +
			'<input type="range" min="0" max="100" step="1" data-yss="alpha" aria-label="背景透明度" /><output></output></label>' +
			'<label class="yss-bgset__field"><span>背景模糊</span>' +
			'<input type="range" min="0" max="' + BG.maxBlur + '" step="1" data-yss="blur" aria-label="背景模糊度" /><output></output></label>' +
			'<p class="yss-bgset__note" data-yss="msg">图片只存在本机浏览器里，不会上传。</p>';
		document.body.appendChild(pop);

		// 把顶栏的深浅色按钮搬进面板，凑成三个圆形按钮。
		// 直接搬元素而不是新建一个：监听器是 initTheme 早就挂好的，搬动不影响；
		// 脚本没跑起来时它照样留在顶栏（那时 class 还是 yss-iconbtn）。
		var themeBtn = document.getElementById("yss-theme-toggle");
		if (themeBtn) {
			themeBtn.className = "yss-roundbtn";
			pop.querySelector(".yss-bgset__row").insertBefore(themeBtn, pop.querySelector('[data-yss="pick"]'));
		}

		function q(name) {
			return pop.querySelector('[data-yss="' + name + '"]');
		}
		var pickBtn = q("pick");
		var clearBtn = q("clear");
		var alphaEl = q("alpha");
		var blurEl = q("blur");
		var msgEl = q("msg");
		var outEls = pop.querySelectorAll("output");
		var msgText = msgEl.textContent;

		var layer = null;    // 图片层，第一次真的要显示时才建
		var current = null;  // 当前图片地址；null = 没设背景
		var busy = false;
		var msgTimer = 0;
		var opt = { alpha: BG.defAlpha, blur: BG.defBlur };

		/* ---- 两个滑块：透明度与模糊度 ---- */

		function applyOpt() {
			// 透明度 = 盖在图片上那层「页面底色」的 alpha（100% 就是完全看不见图）。
			// 颜色由 CSS 按主题给（浅色白纱、深色黑纱），这里只改 alpha。
			document.documentElement.style.setProperty("--yss-userbg-veil-alpha", opt.alpha / 100);
			if (layer) {
				layer.style.filter = opt.blur > 0 ? "blur(" + opt.blur + "px)" : "";
			}
			alphaEl.value = opt.alpha;
			blurEl.value = opt.blur;
			outEls[0].textContent = opt.alpha + "%";
			outEls[1].textContent = opt.blur + "px";
		}

		function readOpt() {
			var stale = false;
			try {
				var raw = JSON.parse(localStorage.getItem(BG.optKey) || "null");
				if (raw && typeof raw === "object" && raw.v === BG.optVer) {
					if (isFinite(raw.alpha)) {
						opt.alpha = Math.max(0, Math.min(100, Math.round(raw.alpha)));
					}
					if (isFinite(raw.blur)) {
						opt.blur = Math.max(0, Math.min(BG.maxBlur, Math.round(raw.blur)));
					}
				} else if (raw) {
					stale = true;   // 旧格式 / 旧默认值：忽略，用新默认值
				}
			} catch (e) {
				/* 坏数据就当没存过 */
			}
			if (stale) {
				saveOpt();   // 顺手把新默认值写回去，下次就跟着新版本号走了
			}
		}

		function saveOpt() {
			try {
				localStorage.setItem(BG.optKey, JSON.stringify({ v: BG.optVer, alpha: opt.alpha, blur: opt.blur }));
			} catch (e) {
				/* 记不住就算了，本次会话里照样有效 */
			}
		}

		/* ---- 图片层 ---- */

		function setState(on) {
			if (on) {
				button.classList.add("is-on");
			} else {
				button.classList.remove("is-on");
			}
			clearBtn.disabled = !on;
			var text = on ? "背景设置（已自定义背景）" : "背景设置";
			button.setAttribute("title", text);
			button.setAttribute("aria-label", text);
		}

		function show(url) {
			if (!layer) {
				layer = document.createElement("div");
				layer.className = "yss-userbg";
				layer.setAttribute("aria-hidden", "true");
				// 插在点阵画布之前：三者同为 z-index:-1，树顺序就是绘制顺序，
				// 于是它落在色晕之上、点阵之下（见 CSS 里的 .yss-userbg）
				var canvas = document.querySelector(".yss-dots");
				document.body.insertBefore(layer, canvas || document.body.firstChild);
			}
			layer.style.backgroundImage = 'url("' + url + '")';
			layer.style.filter = opt.blur > 0 ? "blur(" + opt.blur + "px)" : "";
		}

		// 出错就在面板里说一句，几秒后换回原来的说明文字：不弹窗、不加新控件
		function fail(msg) {
			window.clearTimeout(msgTimer);
			msgEl.textContent = msg;
			msgTimer = window.setTimeout(function () {
				msgEl.textContent = msgText;
			}, 3600);
		}

		function release() {
			if (current && current.indexOf("blob:") === 0) {
				URL.revokeObjectURL(current);
			}
			current = null;
		}

		function pick(file) {
			if (busy || !file) {
				return;
			}
			if (file.type.indexOf("image/") !== 0) {
				fail("请选择图片文件");
				return;
			}
			if (file.size > BG.maxFile) {
				fail("图片太大（上限 32MB）");
				return;
			}
			busy = true;
			pickBtn.disabled = true;
			fail("正在处理图片…");
			bgShrink(file).then(function (blob) {
				return bgSave(blob).then(function (ok) {
					if (!ok) {
						throw new Error("quota");
					}
					return blob;
				});
			}).then(function (blob) {
				busy = false;
				pickBtn.disabled = false;
				release();
				current = URL.createObjectURL(blob);
				show(current);
				setState(true);
				msgEl.textContent = msgText;
				closePop(false);
			}, function (err) {
				busy = false;
				pickBtn.disabled = false;
				fail(err && err.message === "quota" ? "浏览器存不下这张图，换张小一点的" : "这张图读不了，换一张试试");
			});
		}

		function clear() {
			release();
			if (layer) {
				layer.style.backgroundImage = "";
			}
			setState(false);
			// 存储清不掉也不回滚界面：下次打开要是它还在，用户再点一次清除就是了
			bgDrop();
		}

		/* ---- 面板开合 ---- */

		function placePop() {
			var r = button.getBoundingClientRect();
			var vw = document.documentElement.clientWidth || window.innerWidth;
			// 固定定位的参照是初始包含块（不含滚动条），所以用 clientWidth 对齐
			pop.style.right = Math.max(8, Math.round(vw - r.right)) + "px";
			pop.style.top = Math.round(r.bottom + 8) + "px";
		}

		function openPop() {
			placePop();
			pop.hidden = false;
			button.setAttribute("aria-expanded", "true");
			// 把焦点送进面板，键盘用户接着 Tab 就走得下去
			var first = pop.querySelector("button:not(:disabled), input");
			if (first) {
				first.focus();
			}
		}

		function closePop(refocus) {
			if (pop.hidden) {
				return;
			}
			pop.hidden = true;
			button.setAttribute("aria-expanded", "false");
			if (refocus) {
				button.focus();
			}
		}

		button.addEventListener("click", function () {
			if (pop.hidden) {
				openPop();
			} else {
				closePop(true);
			}
		});

		// 点到面板和按钮以外的地方就收起来（pointerdown 早于 click，容器内点选不会误关）
		document.addEventListener("pointerdown", function (e) {
			if (pop.hidden || pop.contains(e.target) || button.contains(e.target)) {
				return;
			}
			closePop(false);
		});

		document.addEventListener("keydown", function (e) {
			if (e.key === "Escape" || e.keyCode === 27) {
				closePop(true);
			}
		});

		window.addEventListener("resize", function () {
			if (!pop.hidden) {
				placePop();
			}
		});

		pickBtn.addEventListener("click", function () {
			input.click();
		});

		clearBtn.addEventListener("click", clear);

		alphaEl.addEventListener("input", function () {
			opt.alpha = parseInt(alphaEl.value, 10) || 0;
			applyOpt();
		});

		blurEl.addEventListener("input", function () {
			opt.blur = parseInt(blurEl.value, 10) || 0;
			applyOpt();
		});

		alphaEl.addEventListener("change", saveOpt);
		blurEl.addEventListener("change", saveOpt);

		input.addEventListener("change", function () {
			var file = input.files && input.files[0];
			// 先取出文件再清空 value：这样连着选同一个文件也会再触发一次 change
			input.value = "";
			pick(file);
		});

		// 先把滑块摆到存下来的值、按钮摆成「没背景」的样子，读到存储再改
		readOpt();
		applyOpt();
		setState(false);
		bgLoadURL().then(function (url) {
			if (url) {
				current = url;
				show(url);
				setState(true);
			}
		});
	}

	/* ----------------------------------------------------------
		11. 面板折射（几何）：让面板背后的**真实内容**被掰弯

		第 9 节那套只扭得动点阵：照片是 DOM 层，canvas 里的逐点位移碰不到它 ——
		所以上传了背景图之后，玻璃的"厚边"依旧是假的。这里换成让浏览器自己扭背板：
		backdrop-filter 可以引用 SVG 滤镜（url(#f)），而 feDisplacementMap 能对
		**整个背板**（照片 + 点阵 + 从面板背后滚过去的正文）做逐像素位移。
		滤镜挂在元素上、跟着元素走，所以滚动时零 JS 开销，视口怎么变都不会错位。

		【这套位移是 LiquidGlassEffect::distortImage 的逐行移植，不要自己"改进"】
		对应的 C++ 就在 Widgets/cpp/LiquidGlassEffect.cpp（nearMap/farMap 那段）。
		要点（照抄，不重新推导）：
		  · R 先夹到 min(宽,高)/2；halfRadius = R/2 **整除**；
		  · nearMap[k] = clamp(int(halfRadius + (1+sin(phase-π/2))·halfRadius·channelScale), 0, min(宽,高)-1)
		    farMap[k]  = clamp(int(sin(phase)·halfRadius·channelScale), 0, R-1)，phase = (k+1)/R·π/2；
		  · 输出像素 (x,y) 的采样位置：x < R 取 nearMap[x]，x >= 宽-R 取 (宽-R)+farMap[x-(宽-R)]，
		    中间取 x 本身；y 同理（同行同列两张表）；四角的块就是行列各查一次表。
		  · 色散 = channelScale 取 {1-d, 1, 1+d}（红/绿/蓝各一张表），色散越大采样越深。
		所以我只做一件事：把"输出像素 → 采样源像素"写成 feDisplacementMap 的位移图
		（偏移 = 源 - 输出，值 = 0.5 + 偏移/R，滤镜 scale = R），算法本身一个字不改。
		三个通道各自一张图 + 取通道相加，就是他们 combineChannels 做的事。

		【能力探测】CSS.supports 只验证语法，所以真画一次：canvas 的 ctx.filter 与
		backdrop-filter 在 Blink 里走的是同一套参考滤镜实现，而 canvas 能读像素，
		于是能确定性地判断"这台浏览器到底会不会渲染参考滤镜"。
		探测里的位移图用 feTurbulence 而不是 feImage：canvas 这条路是同步画的，
		feImage 引的图还没解码完就已经出结果了（实测输出整张全空），而 feTurbulence 是纯计算的。
		探测不过（Safari、较老的 Firefox）就保持第 9 节那套：点阵照扭，照片不扭。
		---------------------------------------------------------- */

	var SVGNS = "http://www.w3.org/2000/svg";

	var LENS = {
		quant: 2,     // 位移图的尺寸量化步长（px）：同尺寸的面板共用一张
		maxMaps: 48,  // 位移图上限，超了就不再建新的（宁可少扭几块）
		res: 0.5,     // 位移图分辨率（相对 CSS 像素）—— 位移场很平滑，半分辨率够用
		// 带宽度相对「形状最短边」的比例。他们的 Demo（正圆）用的是 C++ 里那个上限
		// min(w,h)/2：整条带盖住整个形状，映射恰好把中间一半放大 2 倍（各向同性），
		// 看起来就是凸透镜。我们这里默认也取这个比例，--yss-glass-lens 只作下限。
		ratio: 0.5
	};

	/* 折射带半径。对应他们 C++ 开头那句 clamp，只是在它下面再加一个用户下限：
		R = clamp(max(--yss-glass-lens, 最短边 × ratio), 1, 最短边/2)
		ratio 设 0 就退回「只按 --yss-glass-lens 取，且不超过最短边一半」的旧行为。 */
	function lensRadius(w, h) {
		var minSide = Math.min(w, h);
		var cap = Math.floor(minSide / 2);
		var want = Math.round(minSide * LENS.ratio);
		return Math.max(1, Math.min(cap, Math.max(GLASS.lens, want)));
	}

	var lensSvg = null;   // 存所有滤镜的 <svg>
	var lensOK = null;    // 能力探测结果（null = 还没测过）
	var lensSeq = 0;

	var lensActive = false;   // 滤镜路径生效时，第 9 节的几何与色散都让给滤镜，别扭两遍

	// nearMap / farMap：对应 distortImage 里那两个循环（含 int() 截断与 clamp）。
	// cs = channelScale：1 是不带色散的绿通道，1∓dispersion 是红/蓝通道。
	function lensTabs(R, minSide, cs) {
		var half = Math.floor(R / 2);   // 他们的 halfRadius = liquidDistortRadius / 2，整除
		var near = new Array(R);
		var far = new Array(R);
		for (var k = 0; k < R; k++) {
			var phase = (k + 1) / R * Math.PI / 2;
			near[k] = Math.min(minSide - 1, Math.max(0, Math.floor(half + (1 + Math.sin(phase - Math.PI / 2)) * half * cs)));
			far[k] = Math.min(R - 1, Math.max(0, Math.floor(Math.sin(phase) * half * cs)));
		}
		return { near: near, far: far };
	}

	// 输出像素 p 在某条轴上的采样位置，逐字对应他们那四段分支
	function lensAxis(p, side, R, tab) {
		var idx = Math.floor(p);
		if (idx < 0 || idx > side - 1) {
			return p;
		}
		if (idx < R) {
			return tab.near[idx];
		}
		if (idx >= side - R) {
			return side - R + tab.far[idx - (side - R)];
		}
		return p;
	}

	// 画布上所有像素的 alpha 总量：用来判断"画进去的东西有没有被滤镜动过"
	function lensPaint(g, w, h) {
		var d = g.getImageData(0, 0, w, h).data;
		var sum = 0;
		for (var i = 3; i < d.length; i += 4) {
			sum += d[i];
		}
		return sum;
	}

	function lensSupported() {
		if (lensOK !== null) {
			return lensOK;
		}
		lensOK = false;
		var cv = document.createElement("canvas");
		if (!cv.getContext || !window.CSS || !CSS.supports) {
			return lensOK;
		}
		if (!CSS.supports("backdrop-filter", "url(#a)") && !CSS.supports("-webkit-backdrop-filter", "url(#a)")) {
			return lensOK;
		}
		cv.width = 40;
		cv.height = 40;
		var g = cv.getContext("2d", { willReadFrequently: true });
		if (!g || !g.filter) {
			return lensOK;
		}
		var probe = document.createElementNS(SVGNS, "svg");
		probe.setAttribute("width", "0");
		probe.setAttribute("height", "0");
		probe.style.cssText = "position:fixed;left:-9999px;top:0;width:0;height:0";
		probe.innerHTML = '<defs><filter id="yss-lens-probe" x="0" y="0" width="1" height="1"' +
			' color-interpolation-filters="sRGB" primitiveUnits="userSpaceOnUse">' +
			'<feTurbulence type="fractalNoise" baseFrequency="0.02" numOctaves="1" seed="5" result="m"/>' +
			'<feDisplacementMap in="SourceGraphic" in2="m" scale="200"' +
			' xChannelSelector="R" yChannelSelector="G"/></filter></defs>';
		document.body.appendChild(probe);
		try {
			g.filter = "url(#yss-lens-probe)";
			g.fillStyle = "#000";
			g.fillRect(0, 0, 6, 40);
			// 画的是 6x40 的实心块；被扭过之后不可能还是整整 6*40 个满 alpha 像素
			lensOK = lensPaint(g, 40, 40) !== 6 * 40 * 255;
		} catch (e) {
			lensOK = false;
		}
		document.body.removeChild(probe);
		return lensOK;
	}

	// 位移图：R 通道 = x 方向位移、G 通道 = y 方向位移，0.5 表示不动。
	// 通道值 = 0.5 + 位移/R，配合同样的 scale = R，位移范围正好用满 [R/2, 0]。
	// cs 是这张图用的 channelScale（不带色散时就是 1）。
	function lensMap(w, h, R, cs) {
		var s = LENS.res;
		var mw = Math.max(2, Math.round(w * s));
		var mh = Math.max(2, Math.round(h * s));
		var cv = document.createElement("canvas");
		cv.width = mw;
		cv.height = mh;
		var g = cv.getContext("2d");
		if (!g) {
			return null;
		}
		var im = g.createImageData(mw, mh);
		var d = im.data;
		var tab = lensTabs(R, Math.min(w, h), cs);
		for (var y = 0; y < mh; y++) {
			var py = (y + 0.5) / s;
			var sy = lensAxis(py, h, R, tab);
			for (var x = 0; x < mw; x++) {
				var px = (x + 0.5) / s;
				var sx = lensAxis(px, w, R, tab);
				var i = (y * mw + x) * 4;
				d[i] = Math.round(255 * (0.5 + (sx - px) / R));
				d[i + 1] = Math.round(255 * (0.5 + (sy - py) / R));
				d[i + 2] = 128;
				d[i + 3] = 255;
			}
		}
		g.putImageData(im, 0, 0);
		return cv.toDataURL("image/png");
	}

	// 一张位移图 + 一次位移
	function lensImage(url, w, h, R, tag) {
		var img = document.createElementNS(SVGNS, "feImage");
		img.setAttribute("href", url);
		img.setAttributeNS("http://www.w3.org/1999/xlink", "xlink:href", url);
		img.setAttribute("x", "0");
		img.setAttribute("y", "0");
		img.setAttribute("width", w);
		img.setAttribute("height", h);
		img.setAttribute("preserveAspectRatio", "none");
		img.setAttribute("result", tag + "m");
		var disp = document.createElementNS(SVGNS, "feDisplacementMap");
		disp.setAttribute("in", "SourceGraphic");
		disp.setAttribute("in2", tag + "m");
		disp.setAttribute("scale", R);
		disp.setAttribute("xChannelSelector", "R");
		disp.setAttribute("yChannelSelector", "G");
		disp.setAttribute("result", tag);
		return [img, disp];
	}

	// 只留一个通道（他们 combineChannels 的“提取”那半步）
	function lensChannel(inId, outId, ch) {
		var rows = ["1 0 0 0 0  0 0 0 0 0  0 0 0 0 0  0 0 0 1 0",
			"0 0 0 0 0  0 1 0 0 0  0 0 0 0 0  0 0 0 1 0",
			"0 0 0 0 0  0 0 0 0 0  0 0 1 0 0  0 0 0 1 0"];
		var m = document.createElementNS(SVGNS, "feColorMatrix");
		m.setAttribute("in", inId);
		m.setAttribute("result", outId);
		m.setAttribute("type", "matrix");
		m.setAttribute("values", rows[ch]);
		return m;
	}

	// 通道相加（combineChannels 的“合并”那半步）
	function lensAdd(inA, inB, outId) {
		var c = document.createElementNS(SVGNS, "feComposite");
		c.setAttribute("in", inA);
		c.setAttribute("in2", inB);
		c.setAttribute("operator", "arithmetic");
		c.setAttribute("k2", "1");
		c.setAttribute("k3", "1");
		c.setAttribute("result", outId);
		return c;
	}

	// maps 长度 1（不带色散）或 3（红/绿/蓝各一张表，对应 channelScale = 1±dispersion）
	function lensFilter(id, w, h, R, maps) {
		var f = document.createElementNS(SVGNS, "filter");
		f.setAttribute("id", id);
		// filterUnits 默认是 objectBoundingBox：0,0,1,1 正好是元素自己的框
		f.setAttribute("x", "0");
		f.setAttribute("y", "0");
		f.setAttribute("width", "1");
		f.setAttribute("height", "1");
		// 滤镜内部默认按 linearRGB 算，位移图里那个 128 会被拉歪，必须指名 sRGB
		f.setAttribute("color-interpolation-filters", "sRGB");
		f.setAttribute("primitiveUnits", "userSpaceOnUse");
		var tags = ["dr", "dg", "db"];
		var i;
		for (i = 0; i < maps.length; i++) {
			var pair = lensImage(maps[i], w, h, R, tags[i]);
			f.appendChild(pair[0]);
			f.appendChild(pair[1]);
		}
		if (maps.length === 1) {
			return f;
		}
		for (i = 0; i < 3; i++) {
			f.appendChild(lensChannel(tags[i], tags[i] + "c", i));
		}
		f.appendChild(lensAdd("drc", "dgc", "drg"));
		f.appendChild(lensAdd("drg", "dbc", "tout"));
		return f;
	}

	// 面板在 CSS 里的 backdrop-filter 原值（滤镜像前面那句 url() 是要加上去的）。
	// 只读一次并记在元素上：挂过滤镜之后内联值里已经有 url()，再直接读就把旧的也读进来了；
	// 而每次重建都先摘掉内联值的话，面板会在等位移图解码那几毫秒里"掉"一下玻璃。
	function lensBase(el) {
		if (!el.__yssBase) {
			var cur = el.style.getPropertyValue("backdrop-filter");
			if (cur) {
				el.style.removeProperty("backdrop-filter");
				el.style.removeProperty("-webkit-backdrop-filter");
			}
			var base = getComputedStyle(el).backdropFilter;
			if (!base || base === "none") {
				base = getComputedStyle(el).webkitBackdropFilter;
			}
			if (cur) {
				// 同一个任务里读回来，下一帧之前就还原了，不会被画出来
				el.style.setProperty("backdrop-filter", cur);
				el.style.setProperty("-webkit-backdrop-filter", cur);
			}
			el.__yssBase = base && base !== "none" && base.indexOf("url(") !== 0 ? base : "";
		}
		return el.__yssBase;
	}

	function applyLens() {
		if (!lensSupported()) {
			return;
		}
		var nodes = glassNodes();
		var i;
		if (!lensSvg) {
			lensSvg = document.createElementNS(SVGNS, "svg");
			lensSvg.setAttribute("aria-hidden", "true");
			lensSvg.style.cssText = "position:absolute;left:0;top:0;width:0;height:0;overflow:hidden";
			document.body.appendChild(lensSvg);
		}
		lensSvg.innerHTML = "";
		// 色散也交给滤镜做（三个通道三张表，对应 channelScale = 1∓dispersion），
		// 所以第 9 节不再自己拆通道，否则会叠两遍（见 drawDots 里的 lensActive）。
		var disp = Math.max(0, Math.min(0.5, GLASS.dispersion || 0));   // 他们的 clamp(0, 0.5)
		var scales = disp > 0 ? [1 - disp, 1, 1 + disp] : [1];
		// 【面积预算】超大面板不上滤镜：位移图的滤镜区域就是元素整个盒子，而 feImage
		// 每帧都要把半分辨率位移图重采样到整个盒子。实测同一页（visindigo-module）
		// 一个 992×5032 的面板单独就要 62ms/帧（滚动从 8ms 掉到 70ms），
		// 而其余五个面板加起来不到 4ms —— 换算约 1ms / 20 万像素（三通道位移）。
		// 超预算的面板只留 CSS 的纯 blur：丢的只是左右两条边带（带外本来就是原样），
		// 观感几乎看不出，但滚动立刻回到 120fps。
		var areaBudget = Math.max(1200000, Math.round(window.innerHeight * 1400));
		var boxes = [];
		for (i = 0; i < nodes.length; i++) {
			var r = nodes[i].getBoundingClientRect();
			var base = lensBase(nodes[i]);
			// 没有玻璃（窄屏把模糊降级掉了 / 浏览器不支持）就没必要扭
			if (!base || r.width < 24 || r.height < 24) {
				continue;
			}
			var bw = Math.round(r.width);
			var bh = Math.round(r.height);
			if (bw * bh > areaBudget) {
				// 窗口变小 / 阈值变化前可能上过滤镜，把内联值撤掉让 CSS 的 blur 生效
				nodes[i].style.removeProperty("backdrop-filter");
				nodes[i].style.removeProperty("-webkit-backdrop-filter");
				continue;
			}
			// 他们一开始就把半径夹到最短边的一半，夹完 <= 0 就原图返回
			var br = lensRadius(bw, bh);
			if (br < 4) {
				continue;
			}
			boxes.push({ el: nodes[i], base: base, w: bw, h: bh, R: br, id: null });
		}
		var maps = {};
		var urls = [];
		var count = 0;
		for (i = 0; i < boxes.length; i++) {
			var b = boxes[i];
			var key = Math.round(b.w / LENS.quant) + "x" + Math.round(b.h / LENS.quant) + "x" + b.R;
			if (!maps[key]) {
				if (count >= LENS.maxMaps) {
					continue;
				}
				var made = [];
				for (var ci = 0; ci < scales.length; ci++) {
					var u = lensMap(b.w, b.h, b.R, scales[ci]);
					if (!u) {
						made = null;
						break;
					}
					made.push(u);
				}
				if (!made) {
					continue;
				}
				var id = "yss-lens-" + (lensSeq++);
				lensSvg.appendChild(lensFilter(id, b.w, b.h, b.R, made));
				maps[key] = id;
				urls = urls.concat(made);
				count++;
			}
			b.id = maps[key];
		}

		function glue() {
			var live = false;
			for (var k = 0; k < boxes.length; k++) {
				if (!boxes[k].id) {
					continue;
				}
				// url() 放最前：先掰弯再糊，糊在后面才有"厚玻璃"的钝边
				var v = "url(#" + boxes[k].id + ") " + boxes[k].base;
				boxes[k].el.style.setProperty("backdrop-filter", v);
				boxes[k].el.style.setProperty("-webkit-backdrop-filter", v);
				live = true;
			}
			// 几何与色散都交给滤镜了：第 9 节只剩“画点”，否则点阵会被扭两遍、彩边也会叠两遍。
			// 一块面板都没挂上（全被过滤掉了）时才保留第 9 节那套。
			lensActive = live;
			GLASS.warp = !live;
			if (glassRedraw) {
				glassRedraw();
			}
		}

		if (!urls.length) {
			glue();
			return;
		}
		// 位移图是 data URI：浏览器把它解码完之前，滤镜拿到的是"全透明"的一张图 ——
		// 那等于每个像素都被挪了 -R/2，面板会先错位一下再归位。所以先推进解码队列再挂。
		var left = urls.length;
		for (i = 0; i < urls.length; i++) {
			var pre = new Image();
			pre.onload = pre.onerror = function () {
				if (--left <= 0) {
					glue();
				}
			};
			pre.src = urls[i];
		}
		// 万一一直不 load（不该发生）：半秒后照挂，宁可闪一下也不能没有
		window.setTimeout(function () {
			if (left > 0) {
				left = 0;
				glue();
			}
		}, 500);
	}

	function initLens() {
		if (!lensSupported()) {
			return;   // 退回第 9 节：点阵照扭，照片不扭
		}
		applyLens();
		// 面板尺寸会随字体、窗口变化，重算一次即可（尺寸量化让同尺寸的面板共用一张图）
		var timer = 0;
		function later() {
			window.clearTimeout(timer);
			timer = window.setTimeout(applyLens, 300);
		}
		window.addEventListener("resize", later);
		if (document.fonts && document.fonts.ready && document.fonts.ready.then) {
			document.fonts.ready.then(later);
		}
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
		// 排在点阵之后：背景图层要插在点阵画布前面（同为 z-index:-1，靠树顺序决定上下）
		initUserBackground();
		// 最后挂面板折射滤镜：它接管点阵的几何位移（见第 11 节）
		initLens();
	}

	if (document.readyState === "loading") {
		document.addEventListener("DOMContentLoaded", ready);
	} else {
		ready();
	}
})();
