// wp_nav:导航
;
(function($) {
    $.fn.sudyNav = function() {
        var defaults = {
            vertical: {
                active: '0',
                col: '1'
            }, // 主菜单是否纵向,active 0否 1是  col,主菜单列数
            drop_v: 'down', // 设置子菜单垂直弹出方向，向上或向下,up or down
            drop_w: 'right',
            dir: "y", // 设置子菜单栏目排列方向，x 为横向，y为纵向
            dWidth: '0', // 设置子菜单位置是否以页面宽度参照，否则以主菜单容器宽度(当dir设置为x时有效，即对子菜单横向有效)
            opacity_main: '1',
            opacity_sub: '1'
        };
        return this.each(function() {
                var option = {};
                eval('option = '+($(this).attr("data-nav-config")||{})+';');
                var o = $.extend(true, {}, defaults, option);
                var c = $(this);
                var cW = (o.dWidth == '0') ? c.width() : $(document.body).width();
                var l = c.find(".nav-item");
                var u = c.find("ul");
                var su = u.find("ul");
                var cl = c.children('.nav-item');
                var cu = c.children('.nav-item').children('ul');
                var isIE6 = !-[1, ] && !window.XMLHttpRequest;
                if (o.vertical.active == '1') {
                    c.css({height: 'auto'}).children('.nav-item').css({display: "block", width: (100 / o.vertical.col) + '%', float: 'none'}).children('a').css({display: "block", width: "auto"});
                    if (o.vertical.col !== '1') {
                        c.children(".nav-item").css({display: "inline-block", float: 'left'});
                    }
                    if (o.drop_v == 'up') {
                        u.css({bottom: '0px'});
                    } else {
                        u.css({top: '0px'});
                    }
                    if (o.drop_w == 'left') {
                        u.css({right: "100%"});
                    } else {
                        u.css({left: '100%'});
                    }
                } else {
                    if (o.drop_v == 'up') {
                        su.css({bottom: '0px'});
                    } else {
                        su.css({top: '0px'});
                    }
                    if (o.drop_w == 'left') {
                        su.css({right: "100%"});
                    } else {
                        su.css({left: '100%'});
                    }
                }
                function setOpacity(e, a, w, h) {
                    var mark = e.children('i');
                    if(a!=='-1'&&a!==-1)mark.css({opacity: a});
                    if (isIE6) {
                        mark.css({width: w + 'px', height: h + 'px'});
                    }
                }
                $.each(cl, function() {
                    var clW = $(this).width();
                    var clH = $(this).height();
                    setOpacity($(this), o.opacity_main, clW, clH);
                });
                $.each(u, function() {
                    var sl = $(this).children(".nav-item"), suW = 0, suH = 0, sW = 0, sH = 0;
                    $.each(sl, function() {
                        var slW = $(this).outerWidth(), slH = $(this).outerHeight();
                        suW = suW < slW ? slW : suW;
                        suH += slH;
                        sW += slW;
                        sH = sH < slH ? slH : sH;
                        setOpacity($(this), o.opacity_sub, slW, slH);
                    });
                    if (o.dir == 'x') {
                        var pX = $(this).parent('.nav-item').offset().left + sW;
                        var cX = (o.dWidth == '0') ? (c.offset().left + cW) : cW;
                        var dX = cX - pX;
                        var left = dX < 0 ? dX : 0;
                        $(this).css({width: sW + 'px', height: sH + 'px', left: left + 'px'});
                        var pH = $(this).parent('.nav-item').outerHeight();
                        if (o.drop_v == 'up') {
                            $(this).css({bottom: pH + 'px'});
                        } else {
                            $(this).css({top: pH + 'px'});
                        }
                    } else {
                        $(this).css({width: suW + 'px', height: suH + 'px'});
                        sl.css({display: "block", width: "100%"}).children('a').css({display: "block", width: "auto"});
                        setOpacity(sl, o.opacity_sub, suW, sH);
                    }
                });
                if (o.dir !== 'x' && o.vertical.active !== '1') {
                    $.each(cu, function() {
                        var pH = $(this).parent('.nav-item').outerHeight();
                        if (o.drop_v == 'up') {
                            $(this).css({bottom: pH + 'px'});
                        } else {
                            $(this).css({top: pH + 'px'});
                        }
                        if (o.drop_w == 'left') {
                            $(this).css({right: "0"});
                        } else {
                            $(this).css({left: '0'});
                        }
                    });
                }
                l.hover(function() {
                    $(this).addClass("on").siblings().removeClass("on");
                    $(this).children('a').addClass('parent');
                    $(this).children("ul").css("visibility", "visible").end().siblings().find("ul").css("visibility", "hidden");
                }, function() {
                    $(this).removeClass("on");
                    $(this).children('a').removeClass('parent');
                    $(this).find("ul").css("visibility", "hidden");
                });
        });
    };
})(jQuery);

$(function() {
    $(".wp_nav").sudyNav();
});