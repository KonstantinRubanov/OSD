var controlTableObjects = {}; // Все td объекты принадлежащие данному хосту
var controlJournals = []; // Хранилище журналов проверок КЦ
var controlAudit = [];


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

$(document).ready(function() { 
  $("#img_journal_create_load").hide();
  journalGetList(); 
  
});

/* Действия при загрузке этого js */
$(document).ready(function() { 
    $("#img_control_check_all").hide();
    controlGetList(); 
  } );

  /* Запрос списка хостов */
function controlGetList() {
    "use strict";
    var imgLoader = $("#img_control_loader");
    var imgLoaderStart = $("#img_control_loader_start");
    imgLoaderStart.hide();
    imgLoader.show();
    $("#tbl_control tr").remove();
    sendAjaxRequest("/integrity_control", "get=hosts", function(data) {
        var count = 1;
        var listOfNames = JSON.parse(data);
        imgLoader.hide();
        imgLoaderStart.show();
        if(listOfNames.state === "done") {
          for(var i in listOfNames.data) {
            var vObj = listOfNames.data[i];
            controlJournals[vObj.host] = (vObj.journal == "Error_SSH_Connect")?"Отсутствует соединение SSH":vObj.journal;
            controlAudit[vObj.host] = vObj.audit;
            let errorsName = "Нарушений целостности: ";
            if (parseInt(vObj.errors) < 0) {
              errorsName = "Ошибки создания точки КЦ: ";
              vObj.errors = -parseInt(vObj.errors);
            }
            addRow(count++, vObj.host, vObj.zero_point, vObj.check, errorsName + vObj.errors);
            if(vObj.state === "create_process") {
              var icInstall = controlTableObjects[vObj.host].ic_zero;
              icInstall.text("Создание...");
              var aJournal = controlTableObjects[vObj.host].ic_journal.find('a');
              var spanCheck = controlTableObjects[vObj.host].ic_check.find('span');
              var aCheck = controlTableObjects[vObj.host].ic_check.find('a');
              spanCheck.text("---");
              aCheck.hide();
              aJournal.hide();
              installState(vObj.host);
            } else if(vObj.state === "compare_process"){
              var spanCheck = controlTableObjects[vObj.host].ic_check.find('span');
              var aCheck = controlTableObjects[vObj.host].ic_check.find('a');
              var aJournal = controlTableObjects[vObj.host].ic_journal.find('a');
              aCheck.hide();
              aJournal.hide();
              spanCheck.text('Проверка...');
              compareState(vObj.host);
            }
          }
        }
    });
    function addRow(count, host, zero_point, check, journal) {
        var trObj = $('<tr>');
        var tdObjects = {};
        createTD(count);
        tdObjects.ic_host = createTDHref(host, 'Настройки', 'controlHostSettings("' + host + '")');
        
        var tmpZero = formatDate(zero_point.toString());
        if(tmpZero.match('NaN')) tmpZero = zero_point;
        
        tdObjects.ic_zero = createTD(tmpZero);
        
        var tmpDate = formatDate(check.toString());
        if(tmpDate.match('NaN')) tmpDate = check;
        
        tdObjects.ic_check = createTDHref(tmpDate, 'Запуск', 'controlHostCheck("' + host + '")');
        tdObjects.ic_journal = createTDHref(journal, 'Открыть', 'controlHostJournal("' + host + '")');
        tdObjects.ic_audit = createTDHref("Статистика", "Открыть", 'controlHostAudit("' + host + '")');
        controlTableObjects[host] = tdObjects;
        $("#tbl_control").append(trObj);

        /* Создание элемента td в строке таблицы */
        function createTD(text) {
            var tdObj = $('<td/>', {align: "center", text: text});
            tdObj.appendTo(trObj);
            return tdObj;
        }

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
}

/* Проведение КЦ всех доступных хостов */
function controlCheckAllHosts() {

}

/* Настройки для КЦ хоста */
function controlHostSettings(host) {
  $("#dialog_control_settings").dialog({
    width: 'auto',
    height: 'auto',
    modal: true
  });
  $("#control_div_set_dir img").off("click");
  $("#control_div_ignore_dir img").off("click");
  $("#control_span_hostname").text(host);
  $("#control_btn_cancel").click(function() {
      $("#dialog_control_settings").dialog('close');
  });
  var btnControlAccept = $("#control_btn_accept");
  btnControlAccept.off("click");
  
  var aliasList = [];
  var tbSet = $("#control_tb_set");
  var tbIgnore = $("#control_tb_ignore");
  tbSet.empty();
  tbIgnore.empty();
  sendAjaxRequest("/integrity_control", "get=dirs&host=" + host, function(data) {
    let listOfNames = JSON.parse(data);
    if(listOfNames.state === "error") {
      $("#dialog_control_settings").dialog('close');
      jqcAlert((listOfNames.error_state == "Error_SSH_Connect")?"<center>Отсутствует соединение SSH</center>":listOfNames.error_state);
    } else {
      for(var i in listOfNames.aliases) {
        aliasList.push(listOfNames.aliases[i]);
      }
      var CountNum = 1;
      for(var i in listOfNames.set_dirs) {
        addRow(listOfNames.set_dirs[i].path, listOfNames.set_dirs[i].alias, true, tbSet);
      }
      CountNum = 1;
      for(var i in listOfNames.ignore_dirs) {
        addRow(listOfNames.ignore_dirs[i].path, "", false, tbIgnore);
      }
      $("#ic_audit").prop('checked', listOfNames.is_audit === "true");
    }
  });
  function addRow(name, alias, isAlias, object) {
    var trObj = $('<tr>');
    var tdCountObj = $('<td/>', {align: "center"});
    var tdName = $('<td/>', {align: "left", text: name});
    var tdClose = $('<td/>', {align: "center"});
    tdClose.append($('<img>', {src: "images/delete.png", width: "10px", height: "10px", class: "btn_img_control", on: {click: function(event){ removeControlDir(this);}}}));
    tdCountObj.append($('<img>', {src: isAlias ? "images/plus.png" : "images/minus.png", width: "10px", height: "10px"}));
    trObj.append(tdCountObj);
    trObj.append(tdName);
    if(isAlias) {
      var tdAlias = $('<td/>', {align: "center", text: alias});
      trObj.append(tdAlias);
    }    
    trObj.append(tdClose);
    object.append(trObj);
  }
    
  function removeControlDir(object) {
    $(object).closest("tr").remove();
  }
    
  btnControlAccept.click(function() {
      var dirsSet = {};
      var dirsSetArr = [];
      $('#control_tb_set tr').each(function() {
         var setCount = 0;
         var dirSetObj = {};
         $(this).find('td').each(function(){
             if(setCount === 1) {
              dirSetObj.path = $(this).text();
             } else if (setCount === 2) {
              dirSetObj.alias = $(this).text();
              dirsSetArr.push(dirSetObj);
             }
             setCount++;
         });
      });
      dirsSet.set = dirsSetArr;
      var dirIgnArr = [];
      $('#control_tb_ignore tr').each(function() {
         var setCount = 0;
         $(this).find('td').each(function(){
             if(setCount === 1) {
              dirIgnArr.push($(this).text());
             }
             setCount++;
         });
      });
      dirsSet.ignore = dirIgnArr;
      var str = JSON.stringify(dirsSet, "");
      setTimeout(function() {
        sendAjaxRequest("/integrity_control", "set=dirs&host=" + host + "&set_dirs=" + str + 
            "&is_audit=" + $("#ic_audit").is(':checked'), 
        function(data_set) {
          var listOfNamesSet = JSON.parse(data_set);
          if(listOfNamesSet.state === 'error') { 
            jqcAlert("Установить элементы КЦ не удалось!");
          } else {
            $("#dialog_control_settings").dialog('close');
            installControlPoint(host);
          }
        });
      }, 1000);
  });
  // Для добавления проверяемых каталогов 
  $("#control_div_set_dir img").click(function() {
    showHFWindow(host, function(name) {
      showControlDialogFilesystem(tbSet, name);
    });
  });

  // Для добавления игнорируемых каталогов 
  $("#control_div_ignore_dir img").click(function() {
    showHFWindow(host, function(name) {
      showControlDialogFilesystem(tbIgnore, name);
    });
  });
  // Добавляет очередной каталог 
  function showControlDialogFilesystem(tb_object, name) {
    var isCorrect = true;
    tb_object.children('tr').each(function() {
      var count = 0;
      $(this).find('td').each(function(){
        if(count === 1 && $(this).text() === name) {
          isCorrect = false;
        }
        count++;
      });
    });
    if(isCorrect === true) {
      addRow(name, "PARSEC", tb_object === tbSet, tb_object);
    } else { 
      jqcAlert("Элемент для проверки уже выбран");
    }
  }
}

/* Запрос на проведение сканирования для всех хостов */
function controlCheckAllHosts()
{
  var btnControlCheckHosts = $("#control_scan_hosts_btn");
  var tx_button = btnControlCheckHosts.html(); 
  btnControlCheckHosts.prop("disabled", true);
  var imgControlScanAll = $("#img_control_check_all");
  imgControlScanAll.show();
  btnControlCheckHosts.contents().first().remove();
  Object.keys(controlTableObjects).forEach(function(key) {
    controlHostCheck(key);
  });
  var timerId = setInterval(function() {
    var commonState = true;
    Object.keys(controlTableObjects).forEach(function(key) {
      var scanText = controlTableObjects[key].ic_check.find('span').text();
      if(scanText === "Запуск..." || scanText === "Проверка...") {
        commonState = false;
      }
    });
    if(commonState === true) {
      btnControlCheckHosts.html(tx_button);
      btnControlCheckHosts.prop("disabled", false);
      imgControlScanAll.hide();
      clearInterval(timerId);
    }
   
  }, 2000);
}

/* Запуск КЦ хоста */
function controlHostCheck(host) {
    
       sendAjaxRequest("/action", "resourcebyhost=test_ssh_connect&resourcehost=" + host, function(data) {
    var listOfNames = JSON.parse(data);
    if(listOfNames.data.update != "OK") {
      jqcAlert("<center>Отсутствует соединение SSH</center>");
      return 1;  
    }
    else{
        
  var spanCheck = controlTableObjects[host].ic_check.find('span');
  var aCheck = controlTableObjects[host].ic_check.find('a');
  var aJournal = controlTableObjects[host].ic_journal.find('a');
  let icInstall = controlTableObjects[host].ic_zero;
  aCheck.hide();
  aJournal.hide();
  let spanCheckText = spanCheck.text();
  spanCheck.text('Запуск...');
  if(icInstall.text().length < 5) {
    if(confirm("Точка КЦ не создана. Создать?")) {
      installControlPoint(host);
    } else {
      spanCheck.text(spanCheckText);
      aCheck.show();
      aJournal.show();
    }
  } else {
    sendAjaxRequest("/integrity_control", "point=compare&host=" + host, function(data_main) {
      let dataMain = JSON.parse(data_main);
      if(dataMain.state === 'done') {
        compareState(host);
      }
    });
  }
        
    }
  });   

  
}
//----- Проверка состояния проведения КЦ -----
function compareState(host) {
  var spanCheck = controlTableObjects[host].ic_check.find('span');
  var aCheck = controlTableObjects[host].ic_check.find('a');
  var spanJournal = controlTableObjects[host].ic_journal.find('span');
  var aJournal = controlTableObjects[host].ic_journal.find('a');
  var brJournal = controlTableObjects[host].ic_journal.find('br');
  let DBState = 'Ok'; // Ok - Все хорошо
  var timerId = setInterval(function() {
    sendAjaxRequest("/integrity_control", "point=compare_state&host=" + host, function(data) {
      let listOfNames = JSON.parse(data);
      if(listOfNames.db_error === 'Not_Open_DB') {
        DBState = 'Не получилось открыть БД';
      } else if (listOfNames.db_error === 'No_Query_Exec') {
        DBState = 'Была ошибка в запросе';
      }
      if(listOfNames.state === 'stop') {
        clearInterval(timerId);
        aCheck.show();
        
        var tmpDate = formatDate(listOfNames.date.toString());
        if(tmpDate.match('NaN')) tmpDate = listOfNames.date;
                    
        spanCheck.text(tmpDate);
        controlJournals[host] = (listOfNames.journal == "Error_SSH_Connect")?"Отсутствует соединение SSH":listOfNames.journal;
        
        controlAudit[host] = listOfNames.audit;
                
        spanJournal.text("Нарушений целостности: 0");
        aJournal.show();
        brJournal.show();
        if(DBState != 'Ok') { jqcAlert(DBState); }
      } else if (listOfNames.state === 'process') {
        spanCheck.text('Проверка...');
      } else {  // Ошибка
        clearInterval(timerId);
        aCheck.show();
        
        var tmpDate = formatDate(listOfNames.date.toString());
        if(tmpDate.match('NaN')) tmpDate = listOfNames.date;
                    
        spanCheck.text(listOfNames.journal === "Error_SSH_Connect" ? "Ошибка связи" : tmpDate);
        controlJournals[host] = (listOfNames.journal == "Error_SSH_Connect")?"Отсутствует соединение SSH":listOfNames.journal;
        controlAudit[host] = listOfNames.audit;
        
        spanJournal.text("Нарушений целостности: " + listOfNames.error);
        aJournal.show();
        brJournal.show();
        if(DBState != 'Ok') { jqcAlert(DBState); }
                        
      }
    });
  }, 3000);
}

/* Запрос журнала проведения КЦ хоста */
function controlHostJournal(host) {
    
  sendAjaxRequest("/action", "resourcebyhost=test_ssh_connect&resourcehost=" + host, function(data) {
    var listOfNames = JSON.parse(data);
    if(listOfNames.data.update != "OK") {
      jqcAlert("<center>Отсутствует соединение SSH</center>");
      return 1;  
    }
    else{
      $("#dialog_control_journal").dialog({
    width: 'auto',
    height: 'auto',
    modal: true
      });
      $("#control_journal_text").html(controlJournals[host]);
  }     
  });   
}

/* Запрос журнала аудита КЦ хоста */
function controlHostAudit(host) {
    sendAjaxRequest("/action", "resourcebyhost=test_ssh_connect&resourcehost=" + host, function(data) {
    var listOfNames = JSON.parse(data);
    if(listOfNames.data.update != "OK") {
      jqcAlert("<center>Отсутствует соединение SSH</center>");
      return 1;  
    }
    else{
      $("#dialog_control_audit").dialog({
    width: 'auto',
    height: 'auto',
    modal: true
     });
      $("#control_audit_text").html(controlAudit[host]);
      var table = $(".a_jnl").DataTable();    
    }     
  });     
    
}
//----- Создание точки КЦ -----
function installControlPoint(host) {
  var icInstall = controlTableObjects[host].ic_zero;
  icInstall.text("Создание...");
  var aJournal = controlTableObjects[host].ic_journal.find('a');
  var spanCheck = controlTableObjects[host].ic_check.find('span');
  var aCheck = controlTableObjects[host].ic_check.find('a');
  spanCheck.text("---");
  aCheck.hide();
  aJournal.hide();
  sendAjaxRequest("/integrity_control", "point=install&host=" + host, function(data_install) {
    var listOfNamesInstall = JSON.parse(data_install);
    if(listOfNamesInstall.state === "done") {
      installState(host);
    }
  });
}
//----- Состояние создания точки КЦ -----
function installState(host) {
  var icInstall = controlTableObjects[host].ic_zero;
  var spanJournal = controlTableObjects[host].ic_journal.find('span');
  var aJournal = controlTableObjects[host].ic_journal.find('a');
  var brJournal = controlTableObjects[host].ic_journal.find('br');
  var aCheck = controlTableObjects[host].ic_check.find('a');
  let DBState = 'Ok'; // Ok - Все хорошо
  var timerId = setInterval(function() {
    sendAjaxRequest("/integrity_control", "point=install_state&host=" + host, function(data) {
      var listOfNames = JSON.parse(data);
      if(listOfNames.db_error === 'Not_Open_DB') {
        DBState = 'Не получилось открыть БД';
      } else if (listOfNames.db_error === 'No_Query_Exec') {
        DBState = 'Была ошибка в запросе';
      }
      if(listOfNames.state === 'stop') {
        clearInterval(timerId);
        
        var tmpDate = formatDate(listOfNames.date.toString());
        if(tmpDate.match('NaN')) tmpDate = listOfNames.date;
                    
        icInstall.text(tmpDate);
        if(listOfNames.journal == "abort")
          controlJournals[host] = "Операция создания контрольной точки прервана. См. ошибку в логе asb.log";
        else{
          controlJournals[host] = (listOfNames.journal == "Error_SSH_Connect")?"Отсутствует соединение SSH":listOfNames.journal;
        }
        spanJournal.text("Нарушений целостности: 0");
        aJournal.show();
        brJournal.show();
        aCheck.show();
        if(DBState != 'Ok') { jqcAlert(DBState); }
      } else if (listOfNames.state === 'process') {
        icInstall.text('Создание...');
      } else {  // Ошибка
        clearInterval(timerId);
        icInstall.text('Ошибка!');
        clearInterval(timerId);
        
        if(listOfNames.journal == "abort")
          controlJournals[host] = "Операция создания контрольной точки прервана. См. ошибку в логе asb.log";
        else{        
          controlJournals[host] = (listOfNames.journal == "Error_SSH_Connect")?"Отсутствует соединение SSH":listOfNames.journal;
        }
        listOfNames.error = -parseInt(listOfNames.error);
        spanJournal.text("Ошибки создания точки КЦ: " + listOfNames.error);
        aJournal.show();
        brJournal.show();
        aCheck.show();
        if(DBState != 'Ok') { jqcAlert(DBState); }
      }
    });
  }, 2000);
}
