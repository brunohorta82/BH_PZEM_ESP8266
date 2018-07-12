var map = {
    "config": "",
    "potencia": "Wats",
    "amperagem": "Amperes",
    "voltagem": "Volts",
    "temp": "\u00BAC",
    "contador": ""
};
var limits = {"config": "0", "potencia": "2700", "amperagem": "32", "voltagem": "270", "temp": "180", "contador": "0"};

function loadConfig() {
    var someUrl = "/config";
    $.ajax({
        url: someUrl,
        contentType: "application/json; charset=utf-8",
        dataType: "json",
        success: function (response) {
            console.log(response);

            $('input[name="nodeId"]').val(response[0].nodeId);
            $('input[name="directionCurrentDetection"]').prop("checked", response[0].directionCurrentDetection);
            $('select[name="notificationInterval"] option[value="' + response[0].notificationInterval + '"]').attr("selected", "selected");

            $('input[name="emoncmsApiKey"]').val(response[0].emoncmsApiKey);
            $('input[name="emoncmsUrl"]').val(response[0].emoncmsUrl);
            $('input[name="emoncmsPrefix"]').val(response[0].emoncmsPrefix);

            $('input[name="mqttIpDns"]').val(response[0].mqttIpDns);
            $('input[name="mqttUsername"]').val(response[0].mqttUsername);
            $('input[name="mqttPassword"]').val(response[0].mqttPassword);

            $('input[name="wifiSSID"]').val(response[0].wifiSSID);
            $('input[name="wifiSecret"]').val(response[0].wifiSecret);

            $("#firmwareVersion").text(response[1].firmwareVersion);

            $('select[name="IO_00"] option[value="' + response[0].IO_00 + '"]').attr("selected", "selected");
            $('select[name="IO_02"] option[value="' + response[0].IO_02 + '"]').attr("selected", "selected");
            $('select[name="IO_13"] option[value="' + response[0].IO_13 + '"]').attr("selected", "selected");
            $('select[name="IO_15"] option[value="' + response[0].IO_15 + '"]').attr("selected", "selected");
            $('select[name="IO_16"] option[value="' + response[0].IO_16 + '"]').attr("selected", "selected");

        },
        timeout: 2000
    })
}

function loadReadings() {
    var someUrl = "/readings";
    $.ajax({
        url: someUrl,
        contentType: "application/json; charset=utf-8",
        dataType: "json",
        success: function (response) {
            var i = 0;
            console.log($('#sensors'));
            Object.keys(response).reverse().forEach(function (key) {
                i++;
                if (key !== "config" && key !== "contador") {
                    $('#sensors').append(' <div id="' + key + '" class="GaugeMeter" data-animationstep = 0 data-total="' + limits[key.split("_")[0]] + '" data-used="' + Math.round(response[key]) + '" data-text="' + response[key] + '" data-size="150" data-label_color="#fff" data-used_color="#fff" data-animate_gauge_colors="false" data-width="15" data-style="Semi" data-theme="Red-Gold-Green" data-back="#fff" data-label="' + map[key.split("_")[0]] + '"></div>');
                    $('#' + key).gaugeMeter();
                }
            });
        },
        timeout: 2000
    })
}


$(document).ready(function () {
    $('#node_id').on('keypress', function(e) {
        if (e.which === 32)
            return false;
    });
    loadConfig();
    loadReadings();
    setInterval(loadReadings, 3000);
});