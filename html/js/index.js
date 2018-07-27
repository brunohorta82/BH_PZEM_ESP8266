
    var config = {
        baseUrl: "http://192.168.4.1" /* UNCOMMENT THIS LINE BEFORE SENT TO PRODUCTION */
    };

    var map = {
        "config": "",
        "potencia": "Wats",
        "amperagem": "Amperes",
        "voltagem": "Volts",
        "temp": "\u00BAC",
        "contador": "kWh"
    };
    var mapTitles = {
        "config": "",
        "potencia": "Potência",
        "amperagem": "Corrente",
        "voltagem": "Tensão",
        "temp": "Temperatura",
        "contador": "Contador"
    };
    var mapIcons = {
        "config": "",
        "potencia": "fa-plug",
        "amperagem": "fa-plug",
        "voltagem": "fa-plug",
        "temp": "fa-thermometer-empty",
        "contador": "fa-dot-circle-o"
    };
    var limits = {"config": "0", "potencia": "2700", "amperagem": "32", "voltagem": "270", "temp": "180", "contador": "0"};

function findNetworks(){
    $('#networks').empty();
    $('#status-scan').text('a pesquisar, aguarde...');
    var someUrl = config.baseUrl + "/scan";
        $.ajax({
            url: someUrl,
            contentType: "text/plain; charset=utf-8",
            dataType: "json",
            success: function (response) {
                
            },
            error: function() {

            },complete: function(){

            },
            timeout: 2000
        });
}
function addZeros(i){
    return i < 10 ? "0"+i : i 
}
    function loadConfig() {
        var someUrl = config.baseUrl + "/config";
        $.ajax({
            url: someUrl,
            contentType: "text/plain; charset=utf-8",
            dataType: "json",
            success: function (response) {
                $('input[name="nodeId"]').val(response[0].nodeId);
                $('input[name="nodeId"], input[name="directionCurrentDetection"]').prop('disabled', false);
                $('select[name="notificationInterval"] option[value="' + response[0].notificationInterval + '"]').attr("selected", "selected");
                $('select[name="directionCurrentDetection"] option[value="' + response[0].directionCurrentDetection + '"]').attr("selected", "selected");
                $('input[name="emoncmsApiKey"]').val(response[0].emoncmsApiKey);
                $('input[name="emoncmsUrl"]').val(response[0].emoncmsUrl);
                $('input[name="emoncmsPrefix"]').val(response[0].emoncmsPrefix);
                $('input[name="mqttIpDns"]').val(response[0].mqttIpDns);
                $('input[name="mqttUsername"]').val(response[0].mqttUsername);
                $('input[name="mqttPassword"]').val(response[0].mqttPassword);
                $('input[name="wifiSSID"]').val(response[0].wifiSSID);
                $('input[name="wifiSecret"]').val(response[0].wifiSecret);
                $("#firmwareVersion").text(response[1].firmwareVersion);
                $("#version_lbl").text(response[1].firmwareVersion);
                $('select[name="IO_00"] option[value="' + response[0].IO_00 + '"]').attr("selected", "selected");
                $('select[name="IO_02"] option[value="' + response[0].IO_02 + '"]').attr("selected", "selected");
                $('select[name="IO_13"] option[value="' + response[0].IO_13 + '"]').attr("selected", "selected");
                $('select[name="IO_15"] option[value="' + response[0].IO_15 + '"]').attr("selected", "selected");
                $('select[name="IO_16"] option[value="' + response[0].IO_16 + '"]').attr("selected", "selected");
                $('#ff').prop('disabled', false);
            },
            error: function() {

            },complete: function(){

            },
            timeout: 2000
        });
    }

    function toggleActive(menu) {
        $('.sidebar-menu').find('li').removeClass('active');
        $('.menu-item[data-menu="' + menu +'"]').closest('li').addClass('active');
        $( ".content" ).load(menu + ".html" );
        loadConfig();
    }

    function refreshDashboard() {
        
        let payload = JSON.parse(localStorage.getItem("dashboard"));
        if(!payload)return;
        $('#time-lbl').text(localStorage.getItem('last-update'));
                if($('#sensors .GaugeMeter').length === 0){
                    Object.keys(payload).reverse().forEach(function (key) {
                        if (key !== "config" ) {
                            $('#sensors').append('<div class="col-lg-4 col-md-6 col-xs-12"><div class="info-box bg-aqua"><span class="info-box-icon"><i class="fa '+mapIcons[key.split("_")[0]]+'"></i></span><div class="info-box-content"><span class="info-box-text">'+mapTitles[key.split("_")[0]]+'</span><div id="' + key + '"  class="GaugeMeter" data-animationstep="0" data-total="' + limits[key.split("_")[0]]  + '"  data-size="200" data-label_color="#fff" data-used_color="#fff" data-animate_gauge_colors="false" data-width="15" data-style="Semi" data-theme="Red-Gold-Green" data-back="#fff"  data-label="' + map[key.split("_")[0]] + '"><canvas width="200" height="200"></canvas></div></div></div></div>');
                            $('#' + key).gaugeMeter({used:Math.round(payload[key]),text:payload[key]});
                        }
                    });
                }else{
                    Object.keys(payload).reverse().forEach(function (key) {
                        if (key !== "config" ) {
                            $('#' + key).gaugeMeter({used:Math.round(payload[key]),text:payload[key]});
                        }
                    });
                }
           
    }

    function wifiStatus(response) {
                $('#ssid_lbl').text(response.wifiSSID);
                $('#wifi-icon').attr('title',response.wifiSSID);
                if(response.status){
                    $('#wifi_status').text('ligado');
                    $('#wifi_status_icon').removeClass('text-danger').addClass('text-ok');
                    let percentage = parseFloat(Math.abs(((2*((response.signal)) + 100) / 100))*100).toFixed(0);
                $('#wifi-signal').text(percentage+"%");
                if(percentage > 0 && percentage < 30){
                    $('#wifi-icon')
                    .removeClass('signal-zero')
                    .removeClass('signal-med')
                    .removeClass('signal-good')
                    .removeClass('signal-bad')
                    .addClass('signal-bad');
                }else if(percentage >= 30 && percentage < 61){
                    $('#wifi-icon')
                    .removeClass('signal-zero')
                    .removeClass('signal-med')
                    .removeClass('signal-good')
                    .removeClass('signal-bad')
                    .addClass('signal-med');
                }else if(percentage >= 61){
                    $('#wifi-icon')
                    .removeClass('signal-zero')
                    .removeClass('signal-med')
                    .removeClass('signal-good')
                    .removeClass('signal-bad')
                    .addClass('signal-good');
                }

                }else {
                    $('#wifi_status_icon').removeClass('text-ok').addClass('text-danger')
                    
                    $('#wifi_status').text('desligado');
                }
          
    }

function clearLog(){
   localStorage.setItem('log',""); 
   refreshLogConsole();
}
function selectNetwork(node){
    $('input[name="wifiSSID"]').val(node.split(": ")[5].trim());
    $('input[name="wifiSecret"]').val("").focus();
    
}

function appendNetwork(network){
    if(network === "No networks found"){
        $('#status-scan').text('Não foram encontradas redes wi-fi');
    }else{
        $('#status-scan').text('resultado da pesquisa: ');
        $('#networks').append('<a href="#" class="wifi-node"  data-menu="'+network+'">'+network+'</a><p></p>');
            $('.wifi-node').click(function(e) {
                var node = $(e.currentTarget).data('menu');
                selectNetwork(node);
              
            });
    }
}
function refreshLogConsole(){
    $('#log-box').text(localStorage.getItem('log'))
}

    function loadDefaults() {
        var someUrl = config.baseUrl + "/loaddefaults";
        $.ajax({
            url: someUrl,
            contentType: "text/plain; charset=utf-8",
            dataType: "json",
            success: function (response) {
            },
            timeout: 2000
        });
    }
function getTimestamp(){
    let date = new Date();
    return date.toLocaleDateString('en-GB')+" "+addZeros(date.getHours())+":"+addZeros(date.getMinutes())+":"+addZeros(date.getSeconds());
}
    function reboot() {
        var someUrl = config.baseUrl + "/reboot";
        $.ajax({
            url: someUrl,
            contentType: "text/plain; charset=utf-8",
            dataType: "json",
            success: function (response) {

            },
            timeout: 2000
        });
    }

    $(document).ready(function () {
   
    if (!!window.EventSource) {
      var source = new EventSource('http://192.168.4.1/events');
      
      source.addEventListener('open', function(e) {
        console.log("Events Connected");
        let date = new Date();
        localStorage.setItem("last-update","Atualizado em "+getTimestamp());
        
        refreshDashboard();
      }, false);

      source.addEventListener('error', function(e) {
        if (e.target.readyState != EventSource.OPEN) {
          console.log("Events Disconnected");
        }
      }, false);


      source.addEventListener('dashboard', function(e) {
        localStorage.setItem("dashboard",JSON.stringify(JSON.parse(e.data)));
        refreshDashboard();
      }, false);

        source.addEventListener('wifi', function(e) {
        wifiStatus(JSON.parse(e.data));
      }, false);

        source.addEventListener('wifi-networks', function(e) {
        appendNetwork(e.data);
      }, false);
        source.addEventListener('log', function(e) {
            
            let lastlog =  localStorage.getItem("log") == null ? "" : localStorage.getItem("log");
            localStorage.setItem("log",getTimestamp()+": "+e.data+"\n"+lastlog);
            refreshLogConsole();
            console.log(e.data);
      }, false);
    }
        
        $('#node_id').on('keypress', function(e) {
            if (e.which === 32)
                return false;
        });

        
        $('.menu-item').click(function(e) {
            var menu = $(e.currentTarget).data('menu');
            toggleActive(menu);
          
        });
     
        toggleActive("dashboard");


    });
