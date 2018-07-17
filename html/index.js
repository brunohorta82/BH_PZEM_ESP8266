
var config = {
    baseUrl: "http://192.168.1.86" /* UNCOMMENT THIS LINE BEFORE SENT TO PRODUCTION */
};

var map = {
    "config": "",
    "potencia": "Wats",
    "amperagem": "Amperes",
    "voltagem": "Volts",
    "temp": "\u00BAC",
    "contador": "kWh"
};

var limits = {"config": "0", "potencia": "2700", "amperagem": "32", "voltagem": "270", "temp": "180", "contador": "0"};

function loadConfig() {
    var someUrl = config.baseUrl + "/config";
    $.ajax({
        url: someUrl,
        contentType: "text/plain; charset=utf-8",
        dataType: "json",
        success: function (response) {
            // var response = [{"nodeId":"mynode","notificationInterval":5,"directionCurrentDetection":0,"emoncmsApiKey": "","emoncmsPrefix": "","emoncmsUrl": "","mqttIpDns": "192.168.187.227","mqttUsername": "","mqttPassword": "","wifiSSID": "MAKER_IOT","wifiSecret": "xptoxpto","IO_16": "","IO_13": "13|relay_3|NORMAL","IO_00": "00|relay_2|NORMAL","IO_02": "02|relay_1|NORMAL","IO_15": ""},{"firmwareVersion":1.40}];
            // console.log('Config: ', response);

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
    });
}

function toggleActive(menu) {
    $('.sidebar-menu').find('li').removeClass('active');
    $('.menu-item[data-menu="' + menu +'"]').closest('li').addClass('active');
}

function loadReadings() {
    var someUrl = config.baseUrl + "/readings";
    $.ajax({
        url: someUrl,
        contentType: "text/plain; charset=utf-8",
        dataType: "json",
        success: function (response) {
            if($('#sensors .GaugeMeter').length === 0){
                Object.keys(response).reverse().forEach(function (key) {
                    if (key !== "config" ) {
                        $('#sensors').append(' <div id="' + key + '" class="GaugeMeter" data-animationstep = 0 data-total="' + limits[key.split("_")[0]]  + '" data-size="250" data-label_color="#fff" data-used_color="#fff" data-animate_gauge_colors="false" data-width="15" data-style="Semi" data-theme="Red-Gold-Green" data-back="#fff" data-label="' + map[key.split("_")[0]] + '"></div>');
                        $('#' + key).gaugeMeter({used:Math.round(response[key]),text:response[key]});
                    }
                });
            }else{
                Object.keys(response).reverse().forEach(function (key) {
                    if (key !== "config" ) {
                        $('#' + key).gaugeMeter({used:Math.round(response[key]),text:response[key]});
                    }
                });
            }
        },
        timeout: 2000
    });
}

$(document).ready(function () {
    toggleActive("dashboard");
    loadReadings();
    $( ".content" ).load("dashboard.html" );
    var read = setInterval(loadReadings, 3000);
    $('#node_id').on('keypress', function(e) {
        if (e.which === 32)
            return false;
    });
    loadConfig();

    $('.menu-item').click(function(e) {
        var menu = $(e.currentTarget).data('menu');
        toggleActive(menu);
        if(menu === "dashboard"){
            loadReadings();
             read = setInterval(loadReadings, 3000);
        }else{
            clearInterval(read);
        }
        $( ".content" ).load(menu + ".html" );
    });
});
