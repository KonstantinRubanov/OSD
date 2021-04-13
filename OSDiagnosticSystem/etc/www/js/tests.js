var journalText = {};
var testWidgets = {};

 

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
  $("#img_test_load").hide();
  testsGetHosts(); 
});

/* Запрос списка хостов */
function testsGetHosts() {
  let imgLoader = $("#tst_img_loader");
  let imgLoaderStart = $("#tst_img_loader_start");
  var tableObj = $('#tbl_tests');
  imgLoaderStart.hide();
  imgLoader.show();
  $("#tbl_tests tr").remove();
  let rowCounter = 1;
  sendAjaxRequest("/security", "get=hosts", function(data) {
    imgLoader.hide();
    imgLoaderStart.show();
    let listOfNames = JSON.parse(data);
    for (var i in listOfNames.data) {
      let hostName = listOfNames.data[i].host;
      tableObj.append(addRow(rowCounter++, hostName, 
        listOfNames.data[i].last_start, (listOfNames.data[i].errors === "0" || 
                                        listOfNames.data[i].errors === "") ? 
                                        "Нет ошибок" : "Ошибок: " + listOfNames.data[i].errors));
      journalText[hostName] = (listOfNames.data[i].journal == "Error_SSH_Connect")?"Отсутствует соединение SSH":listOfNames.data[i].journal;
      
      if(listOfNames.data[i].tst_state === "Error_SSH_Connect") {
        testWidgets[hostName].last_start.find('a').hide();
        testWidgets[hostName].last_start.find('span').text("Нет связи");
      } else if (listOfNames.data[i].tst_state === "run") {
        let hrefName = testWidgets[hostName].last_start.find('a');
        let spanName = testWidgets[hostName].last_start.find('span');
        hrefName.hide();
        spanName.text('Идет тестирование...');
        testState(hostName);
      }
    }
  });
  // Добавляет очередную строку с результатом тестирования
  function addRow(count, name, last_start, state) {
    let trObj = $("<tr/>", {width: '100%'}); 
    let tdObjects = {};
    tdObjects.count = createTD(count, "left");
    tdObjects.name = createTD(name, "left");
    
     var tmpCheck = formatDate(last_start.toString());
     if(tmpCheck.match('NaN')) tmpCheck = last_start;
          
    tdObjects.last_start = createTDHref(tmpCheck, "Тестирование", "testStart(\"" + name + "\")");
    tdObjects.state = createTD(state, "center");
    createImageTD("images/state_write.png", "showJournal(\"" + name + "\")");
    testWidgets[name] = tdObjects;
    if(state === 'FAIL') {
        trObj.setAttribute("class", "tr_failed");
    }
    return trObj;
    /* Создание элемента td с текстом */
    function createTD(text, align) {
      let tdObj = $('<td/>', {align: align, text: text === "" ? "---" : text}).appendTo(trObj);
      return tdObj;
    }
    /* Создание элемента td с картинкой (файл/папка) в строке таблицы */
    function createImageTD(img_name, func) {
      let tdObj = $('<td/>', {align: "center", append: $('<img>', { src: img_name, width: 24, height: 24, on: {click: function(event){eval(func);}}}),}).appendTo(trObj);
      return tdObj;
    }
    /* Создание элемента td со ссылкой */
    function createTDHref(text, href_name, func) {
      let tdObj = $('<td/>', {align: "center", append: $('<span>', {text: text === "" ? "---" : text}).add('<br>').add('<a>', {
        href: '#', text: href_name,
        on: { click: function(event) { eval(func);}},
      }),}).appendTo(trObj);
      return tdObj;
    }
  }
}
/* Запуск тестирования всех доступных хостов */
function testsAllHosts() {
  let btnAllHostTest = $("#test_start_btn");
  var tx_button = btnAllHostTest.html();
  let imgTestProcess = $("#img_test_load");
  btnAllHostTest.prop("disabled", true);
  imgTestProcess.show();
  btnAllHostTest.contents().first().remove();
  Object.keys(testWidgets).forEach(function(key) {
    testStart(key);
  });
  let timerId = setInterval(function() {
    let testsState = true;
    Object.keys(testWidgets).forEach(function(key) {
      let spanName = testWidgets[key].last_start.find('span');
      if(spanName.text() === 'Идет тестирование...') {
        testsState = false;
      }
    });
    if(testsState === true) {
      btnAllHostTest.html(tx_button);
      btnAllHostTest.prop("disabled", false);
      imgTestProcess.hide();
    }
   
  }, 2000);
}
/* Запуск тестирования выбранного хоста */
function testStart(host) {
  let hrefName = testWidgets[host].last_start.find('a');
  let spanName = testWidgets[host].last_start.find('span');
  hrefName.hide();
  spanName.text('Идет тестирование...');
  sendAjaxRequest("/security", "test=start&host=" + host, function(data) {
    testState(host);
  });
}
/* Запрос состояния тестирования */
function testState(host) {
  let hrefName = testWidgets[host].last_start.find('a');
  let spanName = testWidgets[host].last_start.find('span');
  let timerId = setInterval(function() {
    sendAjaxRequest("/security", "test=state&host=" + host, function(data) {
      let listOfNames = JSON.parse(data);
      if(listOfNames["state"] === "stop") {
        clearInterval(timerId);
        journalText[host] = (listOfNames.journal == "Error_SSH_Connect")?"Отсутствует соединение SSH":listOfNames.journal;
        testWidgets[host].state.text((listOfNames.errors === "0" || listOfNames.errors === "") ? 
          "Нет ошибок" : "Ошибок: " + listOfNames.errors);
        if(listOfNames.tst_state === "Error_SSH_Connect") {
          hrefName.hide();
          spanName.text('Нет связи');
        } else {
          hrefName.show();
          
         var tmpCheck = formatDate(listOfNames.last_start.toString());
         if(tmpCheck.match('NaN')) tmpCheck = listOfNames.last_start;
                    
          spanName.text(tmpCheck);
        }
      }
    });
  }, 2000);
}

/* Показывает журнал для выбранного хоста */
function showJournal(host) {
  var journalText_ar = journalText[host].split(";");
  var joinText = journalText_ar.join("<br>");
  $("#test_journal_text").html(joinText);
  $("#dialog_test_journal").dialog({
      width: 'auto',
      height: 'auto',
      modal: true
  });
}
