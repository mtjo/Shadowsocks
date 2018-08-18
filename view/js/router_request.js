function RouterRequest() {
    "use strict";
    var authorizeUrl_ = "//account.xiaomi.com/oauth2/authorize";
    var requestUrl_ = "//www.gorouter.info";
    var accessToken_ = "";
    var scope_ = 1000;
    var localUrl_ = "";
    var type_ = "GET";
    var contentType_ = "application/x-www-form-urlencoded; charset=UTF-8";
    var params_stack_ = null;
    var isIFrame_ = false;
    var authorizeError_ = false;
    var authorizeErrorDescription_ = "";

    function updateToken_(token, scope) {
        var count = 0;
        if (token && scope) {
            accessToken_ = token;
            scope_ = scope;
            count = 2;
        } else {
            var url = window.location.href;
            if (url.indexOf("iframe") >= 0) {
                isIFrame_ = true;
            }
            if (url && typeof (url) == "string") {
                var pos = url.indexOf("#");
                if (pos >= 0) {
                    var params = url.substr(pos + 1);
                    var list = params.split("&");
                    for (var i = 0; i < list.length; ++i) {
                        var pair = list[i].split("=");
                        if (pair && pair.length == 2) {
                            if (pair[0] == "access_token") {
                                accessToken_ = pair[1];
                                count += 1;
                            } else if (pair[0] == "scope") {
                                scope_ = pair[1];
                                count += 1;
                            }
                        }
                    }
                }
            }
        }
        if (count == 2) {
            if (parent.window == window) {
                if (params_stack_ && typeof (params_stack_) == "object") {
                    request_(params_stack_);
                }
            } else {
                parent.window.routerRequest.updateToken(accessToken_, scope_);
            }
        }
    }

    function getUrlVars_() {
        var result = {
            localUrl: "",
            redirectUrl: "",
            data: {}
        };
        var href = window.location.href;
        var pos1 = href.indexOf('?');
        var pos2 = href.indexOf('#');
        var params = "";
        if (pos1 < 0 && pos2 < 0) {
            result.localUrl = href;
        } else if (pos1 >= 0) {
            result.localUrl = href.slice(0, pos1);
            params = href.slice(pos1 + 1);
        } else {
            result.localUrl = href.slice(0, pos2);
            params = href.slice(pos2 + 1);
        }
        var paramList = params.split(/\?|#/);
        for (var key in paramList) {
            var hashes = paramList[key].split('&');
            if (!hashes) continue;
            for (var i = 0; i < hashes.length; i++) {
                var hash = hashes[i].split('=');
                if (hash[0] == "redirect") {
                    result.redirectUrl = hash[1];
                } else {
                    result.data[hash[0]] = hash[1]
                }
            }
        }
        return result;
    }

    function authorize_(directUrl, clientId) {
        if (authorizeError_) {
            return;
        }
        var authorizeUrl = authorizeUrl_;
        authorizeUrl += "?client_id=" + clientId;
        authorizeUrl += "&redirect_uri=" + encodeURIComponent(directUrl);
        authorizeUrl += "&response_type=token";
        window.location.href = authorizeUrl;
    }

    function autoAuthorize_(directUrl, clientId) {
        var authorizeUrl = authorizeUrl_;
        authorizeUrl += "?client_id=" + clientId;
        authorizeUrl += "&redirect_uri=" + encodeURIComponent(directUrl);
        authorizeUrl += "&response_type=token";
        authorizeUrl += "&skip_confirm=true";
        var frameId = "tempInnerFrame_";
        var jFrameId = "#" + frameId;
        var frameHtml = "<iframe src='about:blank' id='" + frameId + "' name='" + frameId + "' width='1' height='0' style='display:none'></iframe>";
        if ($(jFrameId).length <= 0) {
            $("body").append(frameHtml);
        }
        $(jFrameId).attr("src", authorizeUrl);
    }

    function makeParams_(data) {
        var params = "",
            first = true;
        for (var key in data) {
            if (first) {
                first = false;
            } else {
                params += "&";
            }
            params += key + "=" + data[key];
        }
        return params;
    }

    function request_(params) {
        if (!params || typeof (params) != "object") return;
        var errorFunction;
        if (typeof (params.error) == "function") {
            errorFunction = params.error;
        } else {
            errorFunction = function () {};
        }
        if (!params.path || typeof (params.path) != "string") {
            errorFunction({
                code: -1,
                msg: "no path given"
            });
            return;
        }
        if (!params.contentType || typeof (params.contentType) != "string") {
            params.contentType = "application/x-www-form-urlencoded; charset=UTF-8";
        }
        if (!params.type || typeof (params.type) != "string") {
            params.type = "GET";
        }
        if (!params.data || typeof (params.data) != "object") {
            errorFunction({
                code: -1,
                msg: "no data given"
            });
            return;
        }
        if (!params.data.clientId) {
            if (params.data.appId && typeof (params.data.appId) == "string") {
                params.data.clientId = params.data.appId;
            } else if (params.data.appid && typeof (params.data.appid) == "string") {
                params.data.clientId = params.data.appid;
            } else {
                errorFunction({
                    code: -1,
                    msg: "no data.appId given"
                });
            }
        }

        if (!params.data.scope || typeof (params.data.scope) != "string") {
            params.data.scope = scope_;
        }
        if (!params.data.token || typeof (params.data.token) != "string") {
            if (accessToken_) {
                params.data.token = accessToken_;
            } else {
                params_stack_ = params;
                var param = makeParams_(params.data);
                var url = localUrl_ + "?redirect=" + params.path + "&iframe=true&" + param;
                autoAuthorize_(url, params.data.clientId);
                return;
            }
        }
        checkSponsor_(params);
        params_stack_ = null;
        $.ajax({
            type: params.type,
            url: requestUrl_ + params.path,
            contentType: params.contentType,
            xhrFields: {
                withCredentials: true
            },
            data: params.data,
            statusCode: {
                401: function () {
                    params_stack_ = params;
                    var param = makeParams_(params.data);
                    var url = localUrl_ + "?redirect=" + params.path + "&iframe=true&" + param;
                    autoAuthorize_(url);
                }
            },
            success: function (data) {
                if (params.success && typeof (params.success) == "function") {
                    params.success(data);
                }
            },
            error: function (xhr, status) {
                errorFunction(status);
            }
        });
    }

    function hasAccessToken_() {
        if (accessToken_) return true;
        return false;
    }

    function needReturn_() {
        return isIFrame_;
    }

    function init_() {
        var params = getUrlVars_();
        if (params.data["access_token"]) accessToken_ = params.data["access_token"];
        if (params.data["scope"]) scope_ = params.data["scope"];
        if (params.data["error"]) {
            authorizeError_ = params.data["error"];
            authorizeErrorDescription_ = params.data["error_description"];
            console.log("error: " + authorizeErrorDescription_);
        }
        localUrl_ = params.localUrl;
        if (!authorizeError_ && params.directUrl) {
            this.request({
                path: params.directUrl,
                data: params.data,
                success: function (data) {
                    console.log("directurl:", params.directUrl, data);
                },
                error: function (data) {
                    console.log("directurl:", data);
                }
            });
        }
    }

    function getAccessToken_() {
        return accessToken_;
    }

    function checkSponsor_(params) {
        if (params.data.clientId != "2882303761517159179" && params.data.clientId != "2882303761517186895") {
            return;
        }
        if (parent.window != window) {
            parent.window.routerRequest.checkSponsor(params);
            return;
        }
        $.ajax({
            type: "GET",
            url: requestUrl_ + "/api-third-party/sponsor",
            contentType: "application/x-www-form-urlencoded; charset=UTF-8",
            xhrFields: {
                withCredentials: true
            },
            data: params.data,
            statusCode: {
                401: function () {
                    params_stack_ = params;
                    var param = makeParams_(params.data);
                    var url = localUrl_ + "?redirect=" + params.path + "&iframe=true&" + param;
                    autoAuthorize_(url);
                }
            },
            success: function (result) {
                var data = JSON.parse(result);
                if (data.code == 0 && data.data.userId != data.data.sponsorId) {
                    window.location.href = "http://app.miwifi.com/tipspage/index.html"
                }
            },
            error: function (xhr, status) {
                console.log(status);
            }
        });
    }

    this.getAccessToken = getAccessToken_;
    this.request = request_;
    this.autoAuthorize = autoAuthorize_;
    this.authorize = authorize_;
    this.updateToken = updateToken_;
    this.hasAccessToken = hasAccessToken_;
    this.needReturn = needReturn_;
    this.checkSponsor = checkSponsor_;
    init_();
    updateToken_();
}
var routerRequest = new RouterRequest();
if (routerRequest.needReturn()) {
    window.location.href = "about:blank";
}