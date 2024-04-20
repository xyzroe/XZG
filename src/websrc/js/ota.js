function sub(t) {
    t = t.value.split("\\\\");
    "" != t ? ($("#updButton").removeAttr("disabled"), localStorage.setItem("beta_feedback", 0)) : $("#updButton").prop(disbl, 1), document.getElementById("file-input").innerHTML = "   " + t[t.length - 1]
}

function sub_zb(t) {
    t = t.value.split("\\\\");
    "" != t ? $("#updButton_zb").removeAttr("disabled") : $("#updButton_zb").prop(disbl, 1), document.getElementById("file-input_zb").innerHTML = "   " + t[t.length - 1]
}

async function fetchReleaseData() {
    var t = await fetch("https://api.github.com/repos/xyzroe/xzg/releases");
    if (t.ok) return await t.json();
    throw new Error("GitHub API request failed: " + t.statusText)
}

$(".flsh_zb").click(function () {
    modalConstructor("flashZB")
});

$("form#esp_upload_form").submit(function (event) {
    event.preventDefault();

    var formData = new FormData($(this)[0]); // Обращаемся к $(this)[0]
    modalConstructor("flashESP", formData);

});

$("form#upload_form_zb").submit(function (t) {
    t.preventDefault();
    t = $("#upload_form_zb")[0];
    t = new FormData(t);
    ZBfwStartEvents(), $.ajax({
        url: "/updateZB",
        type: "POST",
        data: t,
        contentType: !1,
        processData: !1,
        xhr: function () {
            var t = new window.XMLHttpRequest;
            return t.upload.addEventListener("progress", function (t) {
                t.lengthComputable && (t = t.loaded / t.total, $("#prg_zb").html("upload: " + Math.round(100 * t) + "%"), $("#bar_zb").css("width", Math.round(100 * t) + "%"))
            }, !1), t
        },
        success: function (t, e) {
            console.log("success!"), $("#prg_zb").html("Upload and validate completed! <br>Start flashing..."), $("#bar_zb").css("width", "0%")
        },
        error: function (t, e, o) { }
    })
});

$("button#upd_esp_git").click(function () {
    console.log("Update from Git started... Just be patient!"); 
    localStorage.setItem("update_notify", 0);
    //espFlashGitWait();
    modalConstructor("flashESP");
});

$("button#info_esp_git").click(function () {
    modalConstructor("fetchGitReleases");
});
