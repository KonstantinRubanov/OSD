/* eslint-env browser */

var antivirusStates = []; /* Список антивирусов, предназначеных для установки на хосты*/
var dirsForScanHost = ""; /* Список папок для сканирования (разделены ;)*/
var hostSelectedDirs = {};  // Каталоги назначеные для проверки определенного хоста
var runCommand = {};  // Состояние выполнения команды для хоста
var hostObjects = {}; // Все td объекты принадлежащие данному хосту
var isHostListRefresh = false;  // Защита от множественного antivirusGetList

  

function formatDate(date) {
  try{
    var regex = /^[\d-: ]+$/; 
    var ReArray = date.match(regex);
    if(ReArray == null) return date;
    var d = new Date(date),
    month = '' + (d.getMonth() + 1),
    day   = '' + d.getDate(),
    year  = d.getFullYear();  
  
    var hours   = d.getHours();
    var minutes = d.getMinutes();
    var seconds = d.getSeconds();

    if (month.length < 2)   month = '0' + month;
    if (day.length < 2)     day = '0' + day;
    if (hours   < 10) hours = '0' + hours;
    if (minutes < 10) minutes = '0' + minutes;
    if (seconds < 10) seconds = '0' + seconds;
    
    var repData = [day, month, year].join('.') + " " +  [hours, minutes, seconds].join(':');
    return repData;
  }
  catch (e) {
    return date;
  }
}
  

function formatDateShort(date) {
  
  var d = new Date(date),
  month = '' + (d.getMonth() + 1),
  day = '' + d.getDate(),
  year = d.getFullYear();

  if (month.length < 2)   month = '0' + month;
  if (day.length < 2)     day = '0' + day;
  
  return [day, month, year].join('.');
}

/* Действия при загрузке этого js */
$(document).ready(function() { 
  $("#img_antiv_scan_all").hide();
  antivirusGetList(); 
  
} );

/* Запрос списка хостов с антивирусами + доступные дистрибутивы */
function antivirusGetList(func) {
  if(isHostListRefresh === true) {
    return;
  }
  isHostListRefresh = true;
  "use strict";
  var imgLoader = $("#av_img_loader");
  var imgLoaderStart = $("#av_img_loader_start");
  imgLoaderStart.hide();
  imgLoader.show();
  $("#tbl_antivirus tr").remove();
  $(hostObjects).remove();
  sendAjaxRequest("/antivirus", "get=host-list", function(data) {
    var count = 1;
    isHostListRefresh = false;
    var listOfNames = JSON.parse(data);
    imgLoader.hide();
    imgLoaderStart.show();
    for (var i in listOfNames.data) {
        let antivName = listOfNames.data[i].drweb != 0 ? "dr.Web " + listOfNames.data[i].drweb : listOfNames.data[i].kaspersky != 0 ?
            "Kaspersky " + listOfNames.data[i].kaspersky : "---";
        let antivState = listOfNames.data[i].drweb != 0 ? listOfNames.data[i].drweb_state : listOfNames.data[i].kaspersky_state;
        if(listOfNames.data[i].exchange === "discard") {  // Если ошибка связи
          addNullRow(count++, listOfNames.data[i].host);
          continue;
        }
        addRow(count++,
               listOfNames.data[i].host,
               listOfNames.data[i].ip,
               antivName,
               antivState,
               listOfNames.data[i].check,
               listOfNames.data[i].journal,
               listOfNames.data[i].update,
               listOfNames.data[i].scan_type,
               listOfNames.data[i].dir_list);
        hostSelectedDirs[listOfNames.data[i].host] = listOfNames.data[i].dir_list;
        if(listOfNames.data[i].scan_state === "process") {
          let textElement = hostObjects[listOfNames.data[i].host].av_check.find('span');
          let hrefElement = hostObjects[listOfNames.data[i].host].av_check.find('a');
          let stopScanElement = hostObjects[listOfNames.data[i].host].av_check.find('.antiv_stop_scan');
          textElement.text("Сканирование...");
          hrefElement.hide();
          stopScanElement.show();
          processHostScan(listOfNames.data[i].host);
        } else if(listOfNames.data[i].scan_state === "update") {
          antivUpdateState(listOfNames.data[i].host);
        }
    }
    antivirusStates = listOfNames.options;
    if(func != "undefined") {
      func();
    }
  });
  /* Добавление строки в таблицу */
  function addRow(count, host, ip, av_state, av_engine, av_check, av_journal, av_update, scan_type, dir_list) {
    var trObj = document.createElement("TR");
    createTDHTML("<input type=\"checkbox\" class=\"radio\" value=\"" + host + "\" name=\"check_av_list\" />");
    createTD(count);
    createTD(host);    
    var tdObjects = {};
    tdObjects.ip = createTD(ip);
    tdObjects.av_state = createTDHref(av_state, "Настройка", "showAntivOptionsDialog(\"" + host + "\", \"" + scan_type + "\", \"" + dir_list + "\")");
    tdObjects.av_engine = av_engine;
    if(av_state === "---") {
      var tmpCheck = formatDate(av_check.toString());
      if(tmpCheck.match('NaN')) tmpCheck = av_check;
      
      tdObjects.av_check = createTD(av_check.length === 0 ? "---" : tmpCheck);
      tdObjects.av_journal = createTDHref((av_check.length === 0) ? "###" : "Угроз: " + av_journal, "Открыть", "openScanJournal(\"" + host + "\")");
      tdObjects.av_update = createTD("---");
    }
    else {
      var tmpCheck = formatDate(av_check.toString());
      if(tmpCheck.match('NaN')) tmpCheck = av_check;
      
      tdObjects.av_check = createTDHref((av_check.length === 0 ? "---" : tmpCheck), "Запуск", "startHostScan(\"" + host + "\", 0)");
      tdObjects.av_journal = createTDHref((av_check.length === 0) ? "###" : "Угроз: " + av_journal, "Открыть", "openScanJournal(\"" + host + "\")");
      
      var tmpDate = formatDate(av_update.toString());
      if(tmpDate.match('NaN')) tmpDate = av_update;
      
      tdObjects.av_update = createTDHref((av_update === "" ? "---" : tmpDate), "Обновить", "antivUpdate(\"" + host + "\")");
      tdObjects.av_check.append($('<a>', {class: 'antiv_stop_scan',href: '#', text: 'Остановить', on: {click: function(event) { antivScanStop(host)}}}));
      tdObjects.av_check.find('.antiv_stop_scan').hide();
      if(av_engine === "false") {
        tdObjects.av_check.find('a').hide();
        tdObjects.av_check.append($('<span>', {class: 'av_check_disabled', text: 'Отключено'}));
      }
    }
    hostObjects[host] = tdObjects;
    runCommand[host] = false;
    $("#tbl_antivirus").append(trObj);
    /* Создание элемента td в строке таблицы */
    function createTD(text) {
      var tdObj = $('<td/>', {align: "center", text: text});
      tdObj.appendTo(trObj);
      return tdObj;
    }
    function createTDHTML(text) {
      var tdObj = $('<td/>', {align: "center", html: text});
      tdObj.appendTo(trObj);
      return tdObj;
    }
  
   $("input:checkbox[name=check_av_sl]").on('click', function() {
      var $box = $(this);
      if ($box.is(":checked")) {          
        $('input:checkbox[name=check_av_list]').text(function(index, oldText){                                 
          $(this).prop('checked',true);
          return oldText;
        });                    
      } 
      else{        
        $('input:checkbox[name=check_av_list]').text(function(index, oldText){                                 
          $(this).prop('checked',false);
          return oldText;
        });                                 
      }
    });
   
    $("input:checkbox[name=check_av_list]").on('click', function() {              
      var cnt_row = 0;
      var sel_row = 0;
      $('input:checkbox[name=check_av_list]').text(function(index, oldText){   
           var $box = $(this); 
          if ($box.is(":checked")) 
            sel_row++;          
          cnt_row++;
          return oldText;          
      });   
      
      if(sel_row == cnt_row && cnt_row > 0)
        $("input:checkbox[name=check_av_sl]").prop('checked',true);
      if((sel_row != cnt_row && cnt_row > 0) || cnt_row == 0)
        $("input:checkbox[name=check_av_sl]").prop('checked',false);

    });
   
   $('input:checkbox[name=check_av_list]').text(function(index, oldText){                                 
     $(this).prop('checked',true);
     return oldText;
   }); 
   $("input:checkbox[name=check_av_sl]").prop('checked',true);
   $('input:radio[name=check_av_sl_type]').change(function() {        
       var selected = $("input[type='radio'][name='check_av_sl_type']:checked").val();
       if(selected == "check_av_all_policy_lan"){
          $(".is_type_dialog_full_scan").show("slow");
       }
       else
        $(".is_type_dialog_full_scan").hide("slow");       
       
        showAVDialogDirsFullListSettingsReplace();
        return false;
    });
    $(".is_type_dialog_full_scan").hide("slow");  
      
    $('input:radio[name=rb_scan_full_lan]').change(function() {               
        showAVDialogDirsFullListSettingsReplace();
        return false;
    });
    /* Создание элемента td с ссылкой и функцией в строке таблицы */
    function createTDHref(text, href_name, func) {
      var tdObj = $('<td>', {align: "center",
        append: $('<span>', {text: text === "###" ? "---" : text}).add('<br>').add('<a>', {
          href: '#', text: href_name,
          on: { click: function(event) { eval(func);}},
      }),});
      if(text === "###") {
        tdObj.find('a').hide();
        tdObj.find('br').hide();
      }
      tdObj.appendTo(trObj);
      return tdObj;
    }
  }

  /* Добавление строки в таблицу */
  function addNullRow(count, host) {
    var trObj = document.createElement("TR");
    createTD("");
    createTD(count);
    createTD(host);
    var tdObjects = {};
    tdObjects.ip = createTD("Нет связи");
    tdObjects.av_state = createTD("");

    tdObjects.av_check = createTD("");
    tdObjects.av_update = createTD("");
    tdObjects.av_journal = createTD("");

    $("#tbl_antivirus").append(trObj);
    /* Создание элемента td в строке таблицы */
    function createTD(text) {
      var tdObj = $('<td/>', {align: "center", text: text});
      tdObj.appendTo(trObj);
      return tdObj;
    }
  }
}

/* Остановка процесса сканирования */
function antivScanStop(host) {
  let textElement = hostObjects[host].av_check.find('span');
  let hrefElement = hostObjects[host].av_check.find('a');
  let stopScanElement = hostObjects[host].av_check.find('.antiv_stop_scan');
  stopScanElement.hide();
  textElement.text("Остановка...");
  sendAjaxRequest("/antivirus", "scan=host&host=" + host + "&command=stop", function(data) {
    var listOfNames = JSON.parse(data);
      if(listOfNames.state != "done") {
        jqcAlert(listOfNames.state);
      }
  });
}

function antivirusCheckAllClose()
{
   $('#dialog_full_scan').dialog("close");
}

function antivirusCheckAllCloseDirs(){
    $('#dialog_full_scan_dirs').dialog("close");
}

/* Запрос на проведение сканирования для всех хостов */
function antivirusCheckAllHosts()
{
   $('#dialog_full_scan').dialog("close");
   
   stngs_policy = 0;
   stngs_type   = 0;      
  
  $('input:radio[name=check_av_sl_type]:checked').each(function() {
     var stngs_ = $(this).val();
     
     if(stngs_ == "check_av_all_policy_host")
      stngs_policy = 0;
     else if(stngs_ == "check_av_all_policy_lan")
      stngs_policy = 1;
     else
      stngs_policy = 0;
         
  });
  $('input:radio[name=rb_scan_full_lan]:checked').each(function() {
     var stngs_ = $(this).val();
     
     if(stngs_ == "fast")
      stngs_type = 0; 
     else if(stngs_ == "full")
      stngs_type = 1;
     else if(stngs_ == "select")
      stngs_type = 2;
     else
      stngs_type = 0;
  });  
  
   var checked_av = 0; 
   var checked_av_list = [];
      
   $('input:checkbox[name=check_av_list]').text(function(index, oldText){                                 
       oldText = (index + 1).toString();      
       var $box = $(this);
       if ($box.is(":checked")) {
         var group = $box.attr("value");
         /*if(checked_av == 0) checked_av_list = group;
         if(checked_av > 0){
           checked_av_list += "|" + group;
         }*/
         checked_av_list.push(group);
         checked_av = checked_av + 1; 
        }        
        return oldText;
  });
  
  if(checked_av <= 0){ 
      alert("Не выбраны АРМ для проверки");
      return;    
  }

  let btnAllHostScan = $("#antivirus_scan_hosts_btn");
  var tx_button = btnAllHostScan.html(); 
  btnAllHostScan.contents().first().remove();
  btnAllHostScan.html("Остановить");
  
  var btnAntivScanHost = $("#antivirus_scan_hosts_btn");
  //btnAntivScanHost.prop("disabled", true);
  var imgAntivScanAll = $("#img_antiv_scan_all");
  imgAntivScanAll.show();
      
  checked_av_list.forEach(function(item, index, array) {
    if(hostObjects[item].av_engine === "true") { // Только если работает антивирус
      startHostScan(item, stngs_policy);
    }                              
  });
  
  let timerId = setInterval(function() {
    var commonState = true;
    Object.keys(hostObjects).forEach(function(key) {
      var scanText = hostObjects[key].av_check.find('span').text();
      if(scanText === "Запуск..." || scanText === "Сканирование...") {
        commonState = false;
      }
    });
    if(commonState === true) {
      btnAllHostScan.html(tx_button);
      btnAntivScanHost.prop("disabled", false);
      //btnAllHostScan.contents().first().text("Проверка АРМ");
      imgAntivScanAll.hide();
      clearInterval(timerId);
    }
  }, 2000);
//   $("#antivirus_scan_hosts_btn").prop("disabled", false);
}
/* Запрос на проведение сканирования для одного хоста */
function startHostScan(host, isRuleFull)
{  
  let textElement = hostObjects[host].av_check.find('span');
  let hrefElement = hostObjects[host].av_check.find('a');
  let stopScanElement = hostObjects[host].av_check.find('.antiv_stop_scan');
  textElement.text("Запуск...");
  hrefElement.hide();
  stopScanElement.show();
  sendAjaxRequest("/antivirus", "scan=host&host=" + host + "&command=start&scan_policy=" + isRuleFull, function(data) {
    processHostScan(host);
  });
}


function processHostScan(host) {
  let textElement = hostObjects[host].av_check.find('span');
  let hrefElement = hostObjects[host].av_check.find('a');
  let stopScanElement = hostObjects[host].av_check.find('.antiv_stop_scan');
  var jrnlElement = hostObjects[host].av_journal;
  let timerId = setInterval(function() {
    sendAjaxRequest("/antivirus", "scan=host&host=" + host + "&command=state", function(data) {
      var listOfNames = JSON.parse(data);
      if(listOfNames["state"] === "stop") {
        clearInterval(timerId);
        hrefElement.show();
        stopScanElement.hide();
        
        var tmpCheck = formatDate(listOfNames.check.toString());
        if(tmpCheck.match('NaN')) tmpCheck = listOfNames.check;
                    
        textElement.text(tmpCheck);
        if(listOfNames.journal != "") {
          jrnlElement.find('span').text("Угроз: " + listOfNames.journal);
          jrnlElement.find('br').show();
          jrnlElement.find('a').show();
        }
      } else if (listOfNames["state"] === "process") {
        // Может быть в будущем сделаю счетчик
        textElement.text("Сканирование...");
        stopScanElement.show();
      }
      else {  // Какая-то ошибка
        jqcAlert("Потеряна связь с сервером");
        clearInterval(timerId);
        antivirusGetList();
      }
    });
  }, 5000);
}

function openScanJournal(host) 
{
  sendAjaxRequest("/antivirus", "get=scan_journal&host=" + host, function(data) {
    var listOfNames = JSON.parse(data);
    var TextBlock_str = "";
    for(var i in listOfNames.data) {
        if(listOfNames.data[i].string == "Scan interrupted: ScanEngine is not available"){          
           TextBlock_str += "Антивирусная программа не активирована<br>";       
           break;                     
        }
        else
           TextBlock_str += listOfNames.data[i].string + "<br>";
    }
    $("#av_journal_text").html(TextBlock_str);
    $("#dialog_journal").dialog({
      width: '500',
      height: '500',
      modal: true
    });
  });
}


function openScanFullSettings() 
{        
    

  var btnAntivScanHost = $("#antivirus_scan_hosts_btn");
  var imgAntivScanAll = $("#img_antiv_scan_all");
  
  if(!imgAntivScanAll.is(":hidden")){
    alert("Сканирование АРМ будет остановлено");  
    Object.keys(hostObjects).forEach(function(key) {
      antivScanStop(key);
    });
    btnAntivScanHost.prop("disabled",false);
    imgAntivScanAll.hide();
    return;
  }
    
   $("#dialog_full_scan").dialog({
      width: '550',
      height: '440',
      modal: true
    });
   showAVDialogDirsFullListSettings();
}


function OpenDirsFullLan(){
   $("#dialog_full_scan_dirs").dialog({
      width: '550',
      height: '440',
      modal: true
    });
   showAVDialogDirsFullList();
        
}


function antivUpdate(host) 
{
  sendAjaxRequest("/antivirus", "antiv=update&host=" + host + "&command=start", function(data) {
    let listOfNames = JSON.parse(data);
    if(listOfNames.state === "done") {
      antivUpdateState(host);
    } else {
      jqcAlert(listOfNames.error);
    }
  });
}

function antivUpdateState(host) {
  let textElement = hostObjects[host].av_update.find('span');//$(object).parent().find('span');
  let hrefElement = hostObjects[host].av_update.find('a');//$(object).parent().find('a');
  let isEnd = false;
  textElement.text("Запуск...");
  hrefElement.hide();
  let timerId = setInterval(function() {
    sendAjaxRequest("/antivirus", "antiv=update&host=" + host + "&command=state", function(data) {
      let listOfNames = JSON.parse(data);
      if(listOfNames["state"] === "stop") {
        clearInterval(timerId);
        textElement.text(listOfNames.update);
        hrefElement.show();
        antivirusGetList();
      } else if (listOfNames["state"] === "process") {
        // Может быть в будущем сделаю счетчик
        textElement.text("Обновление базы антивируса...");
      }
    });
  }, 5000);
}

function antivSaveTypeAndScan() 
{
  $('#dialog_container').dialog("close");
  var host = $("#tb_name").text();
  sendAjaxRequest("/antivirus", "scan=host_type&host=" + host +
                  "&type=" + $('input[name=rb_scan]:checked', '#host_scan').val() + "&dir_list=" +
                  hostSelectedDirs[host], function(data) {
    antivirusGetList(function() {
      startHostScan(host, 0);
    });
  });
}

//---------------------------------------------------------------------------------------
function showAntivOptionsDialog(host, scan_type, dir_list) {
  let antiv = hostObjects[host].av_state.find('span').text();
  let antivEngine = hostObjects[host].av_engine;
  let btnSetDirList = $("#set_dir_list");
  let cboxAntiv = $("#cbox_antiv");
  let btnAntivInstall = $("#btn_install_antiv");
  let btnAntivRemove = $("#btn_rm_antiv");
  let btnScanTypeReset = $("#btn_scan_type_reset");
  let selectedDirs = hostSelectedDirs[host];
  let antivName = $("#antiv_name");
  let btnApply = $("#btn_save_scan_type");
  let imgScanProcess = $("#img_antiv_scan_dialog");
  $('input[type=radio][name=rb_scan]').off("change");
  btnApply.prop("disabled", true);  // До первых изменений кнопка ПРИМЕНИТЬ недоступна
  let btnScanAndClose = $("#btn_start_scan_and_save");
  btnScanAndClose.prop("disabled", antiv === "---" || antiv === "none" || antivEngine === "false"); // Нет антивируса, сканировать невозможно
  btnScanAndClose.attr("title", antivEngine === "false" ? "Отключено" : "");
  btnScanAndClose.tooltip();
  btnAntivInstall.off("click");
  btnAntivRemove.off("click");
  btnScanTypeReset.off("click");
  btnScanTypeReset.prop("disabled", true);
  btnAntivInstall.prop("disabled", true);
  $("#tb_name").text(host);
  antivName.text(antiv);
  cboxAntiv.empty();
  cboxAntiv.val(0);
  cboxAntiv.change(function(){
    btnAntivInstall.prop("disabled", ($(this).val() === "0"));
  });
  // btnScanStart.prop("disabled", antiv === "---");
  btnAntivRemove.prop("disabled", antiv === "---" || antiv === "none");
  $('#dialog_container').dialog({
      width: 'auto',
      height: 'auto',
      modal: true
  });
  if(antivirusStates.length === 0) {
      addOption(0, "Нет доступных дистрибутивов");
      cboxAntiv.attr("disabled", true);
  }
  else {
    addOption(0, "Выбор антивируса");
    var count = 1;
    for (var i in antivirusStates) {
        addOption(count++, antivirusStates[i].av_version);
    }
    if (runCommand[host] === false) {
      cboxAntiv.attr("disabled", false);
    }
  }
  let rbGroupScan = $("input[type=radio][name=rb_scan]");
  let scanText = hostObjects[host].av_check.find('span').text();
  //----- Когда идет сканирование -----
  if(scanText === "Запуск..." || scanText === "Сканирование...") {
    btnAntivInstall.prop("disabled", true);
    btnAntivRemove.prop("disabled", true);
    btnScanTypeReset.prop("disabled", true);
    cboxAntiv.prop("disabled", true);
    imgScanProcess.show();
    btnScanAndClose.prop("disabled", true);
    btnSetDirList.prop("disabled", true);
    rbGroupScan.attr('disabled', true);
    let timerId = setInterval(function() {
      scanText = hostObjects[host].av_check.find('span').text();
      if(scanText != "Запуск..." && scanText != "Сканирование...") {
        clearInterval(timerId);
        imgScanProcess.hide();
        rbGroupScan.attr('disabled', false);
        btnScanAndClose.prop("disabled", false);
        btnAntivRemove.prop("disabled", antiv === "---" || antiv === "none");
        cboxAntiv.prop("disabled", false);
      }
    }, 1000);
  } else {
    imgScanProcess.hide();
    rbGroupScan.attr('disabled', antiv === "---" || antiv === "none");
    btnSetDirList.prop("disabled", antiv === "---" || antiv === "none");
  }
  $("input[name=rb_scan][value='" + scan_type + "']").prop('checked', true);
  $('input[type=radio][name=rb_scan]').change(function(){
    btnScanTypeReset.prop("disabled", false);
    btnApply.prop("disabled", false);
    btnScanAndClose.prop("disabled", true);
  });
  rbGroupScan.attr('disabled', antiv === "---" || antiv === "none");
  btnSetDirList.prop("disabled", antiv === "---" || antiv === "none");
  /* Добавляет найденные антивирусы в список */
  function addOption(value, text) {
    cboxAntiv.append($('<option>', {value: value, text: text}));
  }
  btnAntivInstall.click(function() {
    rbGroupScan.attr('disabled', true);
    btnScanTypeReset.prop("disabled", true);
    btnSetDirList.prop("disabled", true);
    btnApply.prop("disabled", true);
    installAntiv(host, function() {
      antiv = hostObjects[host].av_state.find('span').text();
      rbGroupScan.attr('disabled', antiv === "---" || antiv === "none");
      btnSetDirList.prop("disabled", antiv === "---" || antiv === "none");
	  if(antiv.indexOf("Kasper") >= 0) { // Обновление после установки антивируса касперского
        antivUpdate(host);
      }
    });
  });
  btnAntivRemove.click(function() {
    btnAntivRemove.prop("disabled", true);
    rbGroupScan.attr('disabled', true);
    btnScanTypeReset.prop("disabled", true);
    btnSetDirList.prop("disabled", true);
    btnApply.prop("disabled", true);
    removeAntiv(host, function() {
      antiv = hostObjects[host].av_state.find('span').text();
      btnAntivRemove.prop("disabled", antiv === "---" || antiv === "none");
      rbGroupScan.attr('disabled', antiv === "---" || antiv === "none");
      btnSetDirList.prop("disabled", antiv === "---" || antiv === "none");
    });
  });
  btnScanTypeReset.click(function() {
    if (scan_type === "") { // Если пришла пыстышка
      scan_type = "fast";
    }
    $("input[name=rb_scan][value='" + scan_type + "']").prop('checked', true);
    hostSelectedDirs[host] = selectedDirs;
    let isBtnApplyDisabled = btnApply.prop("disabled");
    btnApply.prop("disabled", !isBtnApplyDisabled);
    btnScanAndClose.prop("disabled", isBtnApplyDisabled || antiv === "---" || antiv === "none" || antivEngine === "false");
    btnScanTypeReset.prop("disabled", true);
  });
}

function complete() {
    $('#dialog_container').dialog("close");
    clearInterval(timerId);
}

function reinstallAntivirus(host) {
  // Сначала удалим нормально
  removeAntiv(host, function() {
    installAntiv(host);
  });
}

/* Установка выбранного в списке антивируса */
function installAntiv(host, func) {
  if(runCommand[host] === true) { return; } // Выход если выполняется команда установки антивируса
  else { runCommand[host] = true; } 
  var lbAntivName = $("#antiv_name");
  if(lbAntivName.text().indexOf("Kasper") >= 0 || lbAntivName.text().indexOf("dr.Web") >= 0) {
    if(confirm("Переустановить антивирус?")) {
      runCommand[host] = false; // Можно устанавливать заново
      reinstallAntivirus(host);
    }
    return;
  }
  lbAntivName.text("Установка...");
  $("#btn_install_antiv").prop( "disabled", true);
  var textElement = hostObjects[host].av_state.find('span');
  var hrefElement = hostObjects[host].av_state.find('a');
  textElement.text("Установка...");
  hrefElement.hide();
  var cboxAntiv = $("#cbox_antiv");
  var file_run = encodeURIComponent($( "#cbox_antiv option:selected").text());
  sendAjaxRequest("/antivirus", "install=antiv&data=" + host + ":" + file_run, function(data) {
    cboxAntiv.prop( "disabled", true);
    let timerId = setInterval(function() {
    sendAjaxRequest("/antivirus", "install=antiv&host=" + host + "&command=state", function(data) {
      var listOfNames = JSON.parse(data);
      if(listOfNames["state"] === "stop") {
        clearInterval(timerId);
        hrefElement.show();
        textElement.text(listOfNames.av_version);
        lbAntivName.text(listOfNames.av_version);
        let btnScanning = $("#btn_start_scan_and_save");
        if(listOfNames.av_version != "---" && listOfNames.av_version != "none" && $("#btn_save_scan_type").prop("disabled")) {
          btnScanning.prop("disabled", false);
        }
        $("#btn_rm_antiv").prop( "disabled", false);
        $("#btn_install_antiv").prop( "disabled", false);
        runCommand[host] = false; // Можно устанавливать заново
        // btnScanStart.prop("disabled", false);
        antivirusGetList(function() {
          btnScanning.attr("title", hostObjects[host].av_engine === 'true' ? "": "Отключено");
          btnScanning.tooltip();
        });

        cboxAntiv.prop( "disabled", false);
        if(func != "undefined") {
          func();
        }
        return;
      } else if (listOfNames["state"] === "process") {
        // Может быть в будущем сделаю счетчик
        textElement.text("Установка...");
      }
    });
     }, 5000);
  });
}

/* Удаление антивируса */
function removeAntiv(host, func) {
  if(runCommand[host] === true) { return; } // Выход если выполняется команда установки антивируса
  else { runCommand[host] = true; } 
  $("#btn_start_scan_and_save").prop("disabled", true);
  let lbAntivName = $("#antiv_name");
  lbAntivName.text("Удаление...");
  let textElement = hostObjects[host].av_state.find('span');
  let hrefElement = hostObjects[host].av_state.find('a');
  let hrefScanElement = hostObjects[host].av_check.find('a');
  let stopScanElement = hostObjects[host].av_check.find('.antiv_stop_scan');
  let spanDisabled = hostObjects[host].av_check.find('.av_check_disabled');
  spanDisabled.hide();
  hrefScanElement.hide();
  let textUpdate = hostObjects[host].av_update.find('span');
  let hrefUpdate = hostObjects[host].av_update.find('a');
  hrefElement.hide();
  let saveText = textElement.text();
  textElement.text("Удаление...");
    sendAjaxRequest("/antivirus", "remove=antiv&data=" + $("#tb_name").text(), function(data) {
      let remListOfNames = JSON.parse(data);
      if(remListOfNames.state === "done") {
        let timerId = setInterval(function() {
          sendAjaxRequest("/antivirus", "remove=antiv&host_state=" + host, function(data) {
            let listOfNames = JSON.parse(data);
            if(listOfNames["state"] === "stop") {
               clearInterval(timerId);
              lbAntivName.text("---");
              $("#btn_rm_antiv").prop( "disabled", true);
              hrefElement.show();
              runCommand[host] = false;
              // btnScanStart.prop("disabled", true);
              textElement.text("---");
              stopScanElement.hide();
              hrefUpdate.hide();
              textUpdate.text('---');
              if(func != "undefined") {
                func();
              }
              return;
            } else if (listOfNames["state"] === "process") {
            // Может быть в будущем сделаю счетчик
              textElement.text("Удаление...");
            }
          });
        }, 5000);
      } else {
        jqcAlert(remListOfNames.error);
        hrefElement.show();
        textElement.text(saveText);
        lbAntivName.text(saveText);
        $("#btn_rm_antiv").prop( "disabled", false);
      }
  });
}
/* Назначение типа сканирования для выбранного хоста + список сканируемых директорий */
function scanHostType() {
  let antivNameText = $("#antiv_name").text();
  let hostName = $("#tb_name").text();
  sendAjaxRequest("/antivirus", "scan=host_type&host=" + hostName +
                  "&type=" + $('input[name=rb_scan]:checked', '#host_scan').val() + "&dir_list=" +
                  hostSelectedDirs[$("#tb_name").text()], function(data) {
    $("#btn_save_scan_type").prop("disabled", true);
    antivirusGetList(function() {
      let btnScanning = $("#btn_start_scan_and_save");
      btnScanning.prop("disabled", antivNameText === "---" || 
        antivNameText === "none" || hostObjects[hostName].av_engine === "false");
        btnScanning.attr("title", hostObjects[hostName].av_engine === 'true' ? "": "Отключено");
        btnScanning.tooltip();
    });
  });
}
/* Отображение диалога со списком каталогов для проверки */
function showAVDialogDirs() {
  $("#tbl_scan_object tr").remove();
  $("#btn_scan_type_reset").prop("disabled", false);
  $("#btn_save_scan_type").prop("disabled", false);
  $("#dialog_dirs").dialog({
    width: '330px',
    height: 'auto'
  });
  var saveDirList = hostSelectedDirs[$("#tb_name").text()].split(";");
  for(var i in saveDirList) {
    if(saveDirList[i].length > 0) {
      var pathName = saveDirList[i];
      var isDir = true;
      if(pathName[0] === '-') {
        pathName = pathName.substr(1, pathName.length);
        isDir = false;
      }
      addRowDirList(saveDirList[i], isDir);
    }
  }
}


function showAVDialogDirsFullList() {
   $("#full_tbl_scan_object tr").remove();
   sendAjaxRequest("/antivirus", "append_dir=listdirs", function(data) {
   var ListOfNames = JSON.parse(data);
     if(ListOfNames.state == "0"){
       var iDivObj = 0;
       for(i in ListOfNames.data) {                                                       
         addRowDirListFullScan(ListOfNames.data[i], 1)
         iDivObj += 1;
       }               
     }
   });       
}


function showAVDialogDirsFullListSettings() {
   sendAjaxRequest("/antivirus", "settings_dir=get_lan_settings_dir", function(data) {
   var ListOfNames = JSON.parse(data);
     if(ListOfNames.state == "0"){
       if(ListOfNames.data.scan_policy != undefined &&
          ListOfNames.data.scan_type != undefined){
          
           $('input:radio[name=check_av_sl_type]').text(function(index, oldText){   
              var $box = $(this);
              var group = $box.attr("value");
         
              if(ListOfNames.data.scan_policy == 1 && group == "check_av_all_policy_lan"){
                $(this).prop('checked',true);
                $(".is_type_dialog_full_scan").show("slow");
              }
              if(ListOfNames.data.scan_policy == 0 && group == "check_av_all_policy_host"){
                $(this).prop('checked',true);
                $(".is_type_dialog_full_scan").hide("slow");
              }
              
              return oldText;
          }); 
          $('input:radio[name=rb_scan_full_lan]').text(function(index, oldText){   
              var $box = $(this);
              var group = $box.attr("value");
         
              if(ListOfNames.data.scan_type == 0 && group == "fast")
                $(this).prop('checked',true);
              if(ListOfNames.data.scan_type == 1 && group == "full")
                $(this).prop('checked',true);
              if(ListOfNames.data.scan_type == 2 && group == "select")
                $(this).prop('checked',true);
              return oldText;
          });             
         
        }            
     }
   });       
}


function showAVDialogDirsFullListSettingsReplace() {

  stngs_policy = 0;
  stngs_type   = 0;      
  
  $('input:radio[name=check_av_sl_type]:checked').each(function() {
     var stngs_ = $(this).val();
     
     if(stngs_ == "check_av_all_policy_host")
      stngs_policy = 0;
     else if(stngs_ == "check_av_all_policy_lan")
      stngs_policy = 1;
     else
      stngs_policy = 0;
         
  });
  $('input:radio[name=rb_scan_full_lan]:checked').each(function() {
     var stngs_ = $(this).val();
     
     if(stngs_ == "fast")
      stngs_type = 0; 
     else if(stngs_ == "full")
      stngs_type = 1;
     else if(stngs_ == "select")
      stngs_type = 2;
     else
      stngs_type = 0;
  });
  
  sendAjaxRequest("/antivirus", "settings_dir=replace_lan_settings_dir&scan_policy=" + stngs_policy + "&scan_type=" + stngs_type, function(data) {
   var ListOfNames = JSON.parse(data);
     if(ListOfNames.state == "0"){   
        //showAVDialogDirsFullListSettings();
     }
   });       
}

function showAVDialogDirsFullListDelete(name) {
  sendAjaxRequest("/antivirus", "remove_dir=fullscan_remove&data=" + name, function(data) {
  var ListOfNames = JSON.parse(data);  
    if(ListOfNames.state == "0"){
      showAVDialogDirsFullList(); 
    }
  });         
}

/* Отображение диалога со списком каталогов для проверки */
function showAVDialogDirsFullScan() {    
  showHFWindow('127.0.0.1', function(name, isDir) {
    var isCorrect = true;
    $('#full_tbl_scan_object tr').each(function() {
      var count = 0;
      $(this).find('td').each(function(){
        if(count === 1 && $(this).text() === name) {
          isCorrect = false;
        }
        count++;
      });
    });
    if(isCorrect === true) {      
     sendAjaxRequest("/antivirus", "append_dir=fullscan_add&data=" + name, function(data) {                
        var ListOfNames = JSON.parse(data);
        if(ListOfNames.state == "0"){
          showAVDialogDirsFullList(); 
        }
      }); 
          
    } else { 
      jqcAlert("Элемент для проверки уже выбран");
    }
  });
}


/* Добаляет еще одну строку с папкой для проверки */
function addRowDirListFullScan(fullPath, isDir) {
  var trObj = $('<tr>');
  var tdType = $('<td>', {align: "center"});
  tdType.append($('<img>', {src: isDir ? "images/Folder-icon.png" : "images/state_write.png", width: "16px", height: "16px"}));
  var tdDirName = $('<td/>', {align: "left", text: fullPath, class: "antiv_dir_name"});
  var tdDelete = $('<td/>', {align: "center"});
  tdDelete.append($('<img>', {src: "images/delete.png", class: "antv_btn_img", on: {
    click: function(event) {
      showAVDialogDirsFullListDelete(fullPath);
    }
  } }));
  trObj.append(tdType);
  trObj.append(tdDirName);
  trObj.append(tdDelete);
  $('#full_tbl_scan_object').append(trObj);
}


/* Отображение диалога добавления очередного каталога */
function showAVDialogObjAdd() {
  showHFWindow($("#tb_name").text(), function(name, isDir) {
    var isCorrect = true;
    $('#tbl_scan_object tr').each(function() {
      var count = 0;
      $(this).find('td').each(function(){
        if(count === 1 && $(this).text() === name) {
          isCorrect = false;
        }
        count++;
      });
    });
    if(isCorrect === true) {
      addRowDirList(name, isDir);
    } else { 
      jqcAlert("Элемент для проверки уже выбран");
    }
  });
}


/* Добаляет еще одну строку с папкой для проверки */
function addRowDirList(fullPath, isDir) {
  var trObj = $('<tr>');
  var tdType = $('<td>', {align: "center"});
  tdType.append($('<img>', {src: isDir ? "images/Folder-icon.png" : "images/state_write.png", width: "16px", height: "16px"}));
  var tdDirName = $('<td/>', {align: "left", text: fullPath, class: "antiv_dir_name"});
  var tdDelete = $('<td/>', {align: "center"});
  tdDelete.append($('<img>', {src: "images/delete.png", class: "antv_btn_img", on: {
    click: function(event) {removeDirsTR(this);}
  } }));
  trObj.append(tdType);
  trObj.append(tdDirName);
  trObj.append(tdDelete);
  $('#tbl_scan_object').append(trObj);
}

/* Удаление строки из таблицы */
function removeDirsTR(obj) {
  $(obj).closest("tr").remove();
}

/* Применение набора папок для выборочного сканирования */
function setDirList() {
  dirsForScanHost = "";
  $('#dialog_dirs').dialog("close");
  $('#tbl_scan_object tr').each(function(){
    var count = 0;
    $(this).find('td').each(function(){
      if(count === 1) {
        dirsForScanHost += $(this).text() + ";";
      }
      count++;
    });
  });
  dirsForScanHost = dirsForScanHost.substring(0, dirsForScanHost.length - 1);
  hostSelectedDirs[$("#tb_name").text()] = dirsForScanHost;
}
