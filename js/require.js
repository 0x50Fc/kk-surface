
(function(kk){
	var modules = {};
	kk.require = function(path) {
		var m = modules[path];
		if(m === undefined) {
			m = { exports : {} };
			try {
				if(typeof kk.compile == 'function') {
					var fn = kk.compile(path,'(function(module,exports){','})');
					if(typeof fn == 'function') {
						fn = fn();
						if(typeof fn == 'function') {
							fn(m,m.exports);
						} else {
							print("Code Error " + (typeof fn));
						}
					} else {
						print("Not Found " + path);
					}
				} else {
					var code = kk.getString(path);
					var fn = eval("(function(module,exports){" + code + "})");
					if(typeof fn == 'function') {
						fn(m,m.exports);
					}
				}
				print("require " + path);
			} catch(e) {
				print(e.toString());
			}
			modules[path] = m;
		}
		return m.exports;
	};
})(kk);
