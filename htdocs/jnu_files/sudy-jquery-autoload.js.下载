(function() {
    //如果界面上没有加载jquery，执行jquery装载
    if (!window.jQuery) {
        var scripts = document.getElementsByTagName('script');
        var currSrc = scripts[scripts.length - 1].getAttribute("src");
        var parentPath = currSrc.replace(/sudy-jquery-autoload.js/, "");

        //todo，由于jquery的1.9.1以后版本和2.x版本不支持ie6、7、8，
        //判断如果当前是jquery-1.x.min.js 或 jquery-2.x.min.js，并且浏览器是ie6、7、8，直接加载为jquery-1.9.1.min.js
        var jquerySrc = scripts[scripts.length - 1].getAttribute("jquery-src");
        try {
            if (is_lessIE_6_7_8()) {
                jquerySrc = parentPath + "jquery-1.9.1.min.js";
            }
        } catch (e) {
            //如果上述代码异常，采用默认注册的脚本
            jquerySrc = scripts[scripts.length - 1].getAttribute("jquery-src");
        }
        document.write('<script type="text/javascript" src="' + jquerySrc + '"><\/script>');
    }
})();

/**
 * 判断是否为低版本ie（6、7、8）
 * @returns {Boolean}
 */
function is_lessIE_6_7_8() {
    var versionNum = 9;
    if (/Microsoft Internet Explorer/i.test(navigator.appName)) {
        var ver = navigator.appVersion.match(/msie(\s+)?(\d)/i);
        if (ver && Number(ver[2]) < versionNum) {
            return true;
        }
    }
    return false;
}

