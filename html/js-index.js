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
            console.log(response);
            $("#power").text(response.potencia+" W");
            $("#voltage").text(response.voltagem+" V");
            $("#current").text(response.amperagem + " A");
            $("#temperature").text(response.temperatura +" \u00BAC");
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