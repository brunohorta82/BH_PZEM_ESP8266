var map = {"config":"","potencia":"W","amperagem":"A","voltagem":"V","temp":"\u00BAC","contador":""};
function loadConfig() {
    var someUrl = "/config";
    $.ajax({
        url: someUrl,
        contentType: "application/json; charset=utf-8",
        dataType: "json",
        success: function(response) {
            console.log(response);

$('input[name="nodeId"]').val(response.nodeId);
            $('input[name="directionCurrentDetection"]').prop("checked",response.directionCurrentDetection);
            $('select[name="notificationInterval"] option[value="'+response.notificationInterval+'"]').attr("selected","selected");

            $('input[name="emoncmsApiKey"]').val(response.emoncmsApiKey);
            $('input[name="emoncmsUrl"]').val(response.emoncmsUrl);
            $('input[name="emoncmsPrefix"]').val(response.emoncmsPrefix);

            $('input[name="mqttIpDns"]').val(response.mqttIpDns);
            $('input[name="mqttUsername"]').val(response.mqttUsername);
            $('input[name="mqttPassword"]').val(response.mqttPassword);

            $('input[name="wifiSSID"]').val(response.wifiSSID);
            $('input[name="wifiSecret"]').val(response.wifiSecret);

            $("#firmwareVersion").text(response.firmwareVersion);

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
        success: function(response) {

    $('#sensors').empty();
    Object.keys(response).forEach(function (key) {
        $('#sensors').append('<li  class="list-group-item"><h6>'+key+': <span id="'+key+'" class="badge badge-secondary">'+response[key]+' '+map[key.split("_")[0]]+'</span></h6></li>');
    })


        },
        timeout: 2000
    })
}

function handleFirmwareUpdate(){
    window.location.href = "/firmware";
}
$(document).ready(function(){
    loadConfig();
    loadReadings();
    setInterval(loadReadings, 3000);
});