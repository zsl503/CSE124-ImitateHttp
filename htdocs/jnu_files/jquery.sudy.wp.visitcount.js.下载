/**
 * 主要为了实现文章评价功能
 * add by lcfeng
 */
;
(function($) {
    $.fn.WPVisitCount = function(options) {
        var defaults = {};
        var options = $.extend(defaults, options);
        $(this).each(function() {
            var url = $(this).attr("url");
            if (url) {
                initVisitCount(url, $(this));
            }
        });

        //初始化访问次数
        function initVisitCount(url, obj) {
            $.ajax({
                type: "post",
                dataType: "text",
                url: url,
                success: function(result) {
                    if (result !== '' && result !== '0' && result !== '<span>0</span>') {
                        obj.html(result);
                        obj.show();
                    }
                },
                error: function(error) {
                }
            });
        }
    };

    $.fn.WPColListVisitCount = function(options) {
        var defaults = {};
        var options = $.extend(defaults, options);
        var colId = new Array();
        var colIds = "";
        $(this).each(function() {
            var id = $(this).attr("colId");
            var siteId = $(this).attr("siteId");
            id += "_" + siteId;
            colId.push(id);
            colId = unique(colId);
        });
        colIds = colId.join(",");
        if (colIds == "") {
            return;
        }
        var url = "";
        var con = $("script[sudy-wp-context]").attr("sudy-wp-context");
        if (con) {
            con = "/" + con;
            url += con;
        }
//        url += "/_visitcountdisplay?columnIds=" + colIds;
        url += "/_visitcountdisplay?funType=0";
        var type = $("span[class=WP_ColsVisitCount]").attr("type");
        if (type) {
            url += "&type=" + type;
        }
//        url += "&funType=0";
        initColListVisitCount(url, colIds);
    };

    function initColListVisitCount(url, colIds) {
        $.ajax({
            url: url,
            dataType: 'json',
            type: 'post',
            data: {columnIds: colIds},
            success: function(result) {
                if (result !== '' && result !== null) {
                    $.each(result, function(n, value) {
                        var listVisitCount = $("span[colListVisitCountId=wp_colListVisitCount_" + value.columnId + "]");
                        var oldCount = listVisitCount.html();
                        var num = oldCount > value.count ? (oldCount - value.count) : (value.count - oldCount);
                        var total = value.count + num;
                        listVisitCount.html(total);
                        listVisitCount.css('visibility', 'visible');
                    });
                }
            }
        });
    }
    ;

    $.fn.WPArticleReadStatus = function(options) {
        var defaults = {};
        var options = $.extend(defaults, options);
        var artId = new Array();
        var articleIds = "";
        $(this).each(function() {
            var id = $(this).attr("artId");
            artId.push(id);
            artId = unique(artId);
        });
        articleIds = artId.join(",");
        if (articleIds != null && articleIds != '') {
            var url = "";
            var con = $("script[sudy-wp-context]").attr("sudy-wp-context");
            if (con) {
                con = "/" + con;
                url += con;
            }
            url += "/_visitcountdisplay?articleIds=" + articleIds;
            var siteId = $("script[sudy-wp-siteId]").attr("sudy-wp-siteId");
            if (siteId) {
                url += "&siteId=" + siteId;
            }
            url += "&funType=1";
            initReadStatus(url);
        }
    };

    function unique(arr) {
        var res = [];
        var json = {};
        for (var i = 0; i < arr.length; i++) {
            if (!json[arr[i]]) {
                res.push(arr[i]);
                json[arr[i]] = 1;
            }
        }
        return res;
    }

    function initReadStatus(url) {
        $.ajax({
            url: url,
            dataType: 'text',
            type: "post",
            success: function(result) {
                if (result !== "") {
                    var artIds = result.split(",");
                    for (var i = 0; i < artIds.length; i++) {
                        $("span[readStatusId=wp_artReadStauts_" + artIds[i] + "]").addClass("wp_artReadStatus_unread");
                    }
                }
            }
        });
    }
    ;

    $.fn.WPListVisitCount = function(options) {
        var defaults = {};
        var options = $.extend(defaults, options);
        var artId = new Array();
        var articleIds = "";
        $(this).each(function() {
            var id = $(this).attr("artId");
            artId.push(id);
            artId = unique(artId);
        });
        articleIds = artId.join(",");
        if (articleIds != null && articleIds != '') {
            var url = "";
            var con = $("script[sudy-wp-context]").attr("sudy-wp-context");
            if (con) {
                con = "/" + con;
                url += con;
            }
            url += "/_visitcountdisplay?articleIds=" + articleIds;
            var siteId = $("script[sudy-wp-siteId]").attr("sudy-wp-siteId");
            if (siteId) {
                url += "&siteId=" + siteId;
            }
            var type = $("span[class=wp_listVisitCount]").attr("type");
            if (type) {
                url += "&type=" + type;
            }
            url += "&funType=0";
            initListVisitCount(url);
        }
    };

    function initListVisitCount(url) {
        $.ajax({
            url: url,
            dataType: 'json',
            type: 'post',
            success: function(result) {
                if (result !== '' && result !== null) {
                    $.each(result, function(n, value) {
                        var listVisitCount = $("span[listVisitCountId=wp_listVisitCount_" + value.articleId + "]");
                        listVisitCount.html(value.count);
                        listVisitCount.css('visibility', 'visible');
                    });
                }
            }
        });
    }
    ;
})(jQuery);

$(document).ready(function() {
    if ($('.WP_VisitCount').hasClass('WP_VisitCount')) {
        $('.WP_VisitCount').WPVisitCount();
    }
    if ($('.wp_artReadStatus').hasClass('wp_artReadStatus')) {
        $('.wp_artReadStatus').WPArticleReadStatus();
    }
    if ($('.wp_listVisitCount').hasClass('wp_listVisitCount')) {
        $('.wp_listVisitCount').WPListVisitCount();
    }
    if ($('.WP_ColsVisitCount').hasClass('WP_ColsVisitCount')) {
        $('.WP_ColsVisitCount').WPColListVisitCount();
    }
}); 

//下面这个根据ip获取地址的接口坏了，如果要使用，需要另找接口
//$().ready(function() {
//    $.ajax({
//        type: 'get',
//        dataType: 'jsonp',
//        //url: "http://ip.chinaz.com/ajaxsync.aspx?at=ip&ip=" + ip, //这里是url
//        url: "http://ip.chinaz.com/getip.aspx",//调用URL返回ip和来访地址 返回值{ip:'xxx.xxx.xxx.xxx',address:'江苏省南京市 电信'}
//        success: function(data) {
//            var address = data.address.replace("电信", "").replace("移动","").replace("联通","");//取出地址并且将里面的运营商去掉
//            document.cookie = "ipAddressName=" + escape(address) + ";path=/; ";//将地址信息塞入cookie
//        },
//        error: function() {
//       
//        }
//    });
//})