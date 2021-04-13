/* Данная функция создаёт кроссбраузерный объект XMLHTTP */
function getXmlHttp() {
  var xmlhttp;
  try {
    xmlhttp = new ActiveXObject("Msxml2.XMLHTTP");
  } catch (e) {
  try {
    xmlhttp = new ActiveXObject("Microsoft.XMLHTTP");
  } catch (E) {
    xmlhttp = false;
  }
  }
  if (!xmlhttp && typeof XMLHttpRequest!='undefined') {
    xmlhttp = new XMLHttpRequest();
  }
  return xmlhttp;
}

function testStart() {
  var xmlhttp = getXmlHttp(); // Создаём объект XMLHTTP
  xmlhttp.open('POST', '/security', true); // Открываем асинхронное соединение
  xmlhttp.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded');
  xmlhttp.send("tests=start");
  xmlhttp.onreadystatechange = function() { // Ждём ответа от сервера
    if (xmlhttp.readyState == 4) { // Ответ пришёл
      if(xmlhttp.status == 200) { // Сервер вернул код 200 (что хорошо)
        var journals_div = document.getElementById("tests_block");
        while (journals_div.firstChild) {
          journals_div.removeChild(journals_div.firstChild);
        }
        var ListOfNames = JSON.parse(xmlhttp.responseText);
        for(i in ListOfNames.data) {
          var nameTest = document.createElement("SPAN");
          var nameTestText = document.createTextNode('Лог журнала:  ' + ListOfNames.data[i].name +
              ' ##### ' + ListOfNames.data[i].state);
          nameTest.appendChild(nameTestText);
          journals_div.appendChild(nameTest);
          var bbb = document.createElement("BR");
          journals_div.appendChild(bbb);
        }
        xmlhttp.send(ListOfNames);
      }
      else if (xmlhttp.status == 401) { // Нет авторизации
        location.href = '/';
      }
      else {
        //обработка ошибки
        alert('error: ' + this.status ? this.statusText : 'query failed');
        return;
      }
    }
  };
}

function backupAldStart() {
  var xmlhttp = getXmlHttp(); // Создаём объект XMLHTTP
  xmlhttp.open('POST', '/ald', true); // Открываем асинхронное соединение
  xmlhttp.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded');
  xmlhttp.send("backup=save&file=new");
  xmlhttp.onreadystatechange = function() { // Ждём ответа от сервера
    if (xmlhttp.readyState == 4) { // Ответ пришёл
      if(xmlhttp.status == 200) { // Сервер вернул код 200 (что хорошо)
        var testBlock = document.getElementById("tests_block");
        while (testBlock.firstChild) {
          testBlock.removeChild(testBlock.firstChild);
        }
        JSON.parse(xmlhttp.responseText, function(v, k) {
          var nameTest = document.createElement("SPAN");
          var nameTestText = document.createTextNode('Резервная копия от ' + k);
          nameTest.appendChild(nameTestText);
          testBlock.appendChild(nameTest);
          var bbb = document.createElement("BR");
          testBlock.appendChild(bbb);
        });
      }
      else if (xmlhttp.status == 401) { // Нет авторизации
        location.href = '/';
      }
    }
  };
}

function userlistAld() {
  var xmlhttp = getXmlHttp(); // Создаём объект XMLHTTP
  xmlhttp.open('POST', '/ald', true); // Открываем асинхронное соединение
  xmlhttp.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded');
  xmlhttp.send("usersAld=userlist");
  xmlhttp.onreadystatechange = function() { // Ждём ответа от сервера
    if (xmlhttp.readyState == 4) { // Ответ пришёл
      if(xmlhttp.status == 200) { // Сервер вернул код 200 (что хорошо)
        var userlist_btn = document.getElementById("userlist_btn");
        while (userlist_btn.firstChild) {
          userlist_btn.removeChild(userlist_btn.firstChild);
        }
        var ListOfNames = JSON.parse(xmlhttp.responseText, function(key, value) {
          var nameTest = document.createElement("SPAN");
          var nameTestText = document.createTextNode('Резервная копия от ' + value);
          nameTest.appendChild(nameTestText);
          userlist_btn.appendChild(nameTest);
          var bbb = document.createElement("User" + key);
          userlist_btn.appendChild(bbb);
        });
        xmlhttp.send(ListOfNames);
      }
      else if (xmlhttp.status == 401) { // Нет авторизации
        location.href = '/';
      }
      else {
        //обработка ошибки
        alert('error: ' + this.status ? this.statusText : 'query failed');
        return;
      }
    }
  };
}


function journalGetData() {
  var xmlhttp = getXmlHttp(); // Создаём объект XMLHTTP
  xmlhttp.open('POST', '/journals', true); // Открываем асинхронное соединение
  xmlhttp.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded');
  xmlhttp.send("get=data");
  xmlhttp.onreadystatechange = function() { // Ждём ответа от сервера
    if (xmlhttp.readyState == 4) { // Ответ пришёл
      if(xmlhttp.status == 200) { // Сервер вернул код 200 (что хорошо)
        var journals_div = document.getElementById("tests_block");
        while (journals_div.firstChild) {
          journals_div.removeChild(journals_div.firstChild);
        }
        var ListOfNames = JSON.parse(xmlhttp.responseText);
        for(i in ListOfNames.data) {
          var nameTest = document.createElement("SPAN");
          var nameTestText = document.createTextNode('Лог журнала:  ' + ListOfNames.data[i].type + ': ' + ListOfNames.data[i].start_date + ' ' + ListOfNames.data[i].stop_date);
          nameTest.appendChild(nameTestText);
          journals_div.appendChild(nameTest);
          var bbb = document.createElement("BR");
          journals_div.appendChild(bbb);
        }
        xmlhttp.send(ListOfNames);
      }
      else if (xmlhttp.status == 401) { // Нет авторизации
        location.href = '/';
      }
      else {
        //обработка ошибки
        alert('error: ' + this.status ? this.statusText : 'query failed');
        return;
      }
    }
  };
}

function tst_testStart() {
  var x = document.createElement("SPAN");
  var t = document.createTextNode("This is a span \telement.");
  x.appendChild(t);
  document.body.appendChild(x);
}

function logPrint() {
  var logStart_date = document.getElementById("log_start_date");
  var logStop_date = document.getElementById("log_stop_date");
  var xmlhttp = getXmlHttp(); // Создаём объект XMLHTTP
  xmlhttp.open('POST', '/journals', true); // Открываем асинхронное соединение
  xmlhttp.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded');
  xmlhttp.send("get=print&&start_date=" + logStart_date.value + "&&stop_date=" + logStop_date.value);
  xmlhttp.onreadystatechange = function() { // Ждём ответа от сервера
    if (xmlhttp.readyState == 4) { // Ответ пришёл
      if(xmlhttp.status == 200) { // Сервер вернул код 200 (что хорошо)
        var journals_div = document.getElementById("tests_block");
        while (journals_div.firstChild) {
          journals_div.removeChild(journals_div.firstChild);
        }
        var ListOfNames = JSON.parse(xmlhttp.responseText);
        for(i in ListOfNames.data) {
          var nameTest = document.createElement("SPAN");
          var nameTestText = document.createTextNode(ListOfNames.data[i].string);
          nameTest.appendChild(nameTestText);
          journals_div.appendChild(nameTest);
          var bbb = document.createElement("BR");
          journals_div.appendChild(bbb);
        }
        xmlhttp.send(ListOfNames);
      }
      else if (xmlhttp.status == 401) { // Нет авторизации
        location.href = '/';
      }
    }
  }
}

function createJournalArch() {
  var logStart_date = document.getElementById("log_start_date");
  var logStop_date = document.getElementById("log_stop_date");
  var xmlhttp = getXmlHttp(); // Создаём объект XMLHTTP
  xmlhttp.open('POST', '/journals', true); // Открываем асинхронное соединение
  xmlhttp.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded');
  xmlhttp.send("set=archive&&start_date=" + logStart_date.value + "&&stop_date=" + logStop_date.value);
  xmlhttp.onreadystatechange = function() { // Ждём ответа от сервера
    if (xmlhttp.readyState == 4) { // Ответ пришёл
      if(xmlhttp.status == 200) { // Сервер вернул код 200 (что хорошо)

      }
      else if (xmlhttp.status == 401) { // Нет авторизации
        location.href = '/';
      }
    }
  }
}

function OutToPrinter() {
  var prtContent = document.getElementById("tests_block");
  var WinPrint = window.open('','','left=50,top=50,width=800,height=640,toolbar=0,scrollbars=1,status=0');
  WinPrint.document.write('');
  WinPrint.document.write(prtContent.innerHTML);
  WinPrint.document.write('');
  WinPrint.document.close();
  WinPrint.focus();
  WinPrint.print();
  WinPrint.close();
  prtContent.innerHTML=strOldOne;
}

function GetHostsList() {
  var xmlhttp = getXmlHttp(); // Создаём объект XMLHTTP
  xmlhttp.open('POST', '/antivirus', true); // Открываем асинхронное соединение
  xmlhttp.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded');
  xmlhttp.send("get=host-list");
  xmlhttp.onreadystatechange = function() { // Ждём ответа от сервера
    if (xmlhttp.readyState == 4) { // Ответ пришёл
      if(xmlhttp.status == 200) { // Сервер вернул код 200 (что хорошо)
        var antiv_div = document.getElementById("tests_block");
        while (antiv_div.firstChild) {
          antiv_div.removeChild(antiv_div.firstChild);
        }
        var ListOfNames = JSON.parse(xmlhttp.responseText);
        for(i in ListOfNames.data) {
          var nameTest = document.createElement("SPAN");
          var nameTestText = document.createTextNode(ListOfNames.data[i].host + " dr.Web=" + ListOfNames.data[i].drweb + " Kaspersky=" + ListOfNames.data[i].kaspersky);
          nameTest.appendChild(nameTestText);
          antiv_div.appendChild(nameTest);
          var bbb = document.createElement("BR");
          antiv_div.appendChild(bbb);
        }

        for(i in ListOfNames.options) {
          var nameTest = document.createElement("SPAN");
          var nameTestText = document.createTextNode(ListOfNames.options[i].av_type + " ver=" + ListOfNames.options[i].av_version);
          nameTest.appendChild(nameTestText);
          antiv_div.appendChild(nameTest);
          var bbb = document.createElement("BR");
          antiv_div.appendChild(bbb);
        }
        xmlhttp.send(ListOfNames);
      }
      else if (xmlhttp.status == 401) { // Нет авторизации
        location.href = '/';
      }
      else {
        //обработка ошибки
        alert('error: ' + this.status ? this.statusText : 'query failed');
        return;
      }
    }
  };
}

function InstallAntivOnHost() {
  var xmlhttp = getXmlHttp(); // Создаём объект XMLHTTP
  xmlhttp.open('POST', '/antivirus', true); // Открываем асинхронное соединение
  xmlhttp.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded');
  var antiv_input = document.getElementById("host_name");
  xmlhttp.send("install=antiv&&data=" + antiv_input.value);
  xmlhttp.onreadystatechange = function() { // Ждём ответа от сервера
    if (xmlhttp.readyState == 4) { // Ответ пришёл
      if(xmlhttp.status == 200) { // Сервер вернул код 200 (что хорошо)
      }
      else if (xmlhttp.status == 401) { // Нет авторизации
        location.href = '/';
      }
      else {
        //обработка ошибки
        alert('error: ' + this.status ? this.statusText : 'query failed');
        return;
      }
    }
  };
}
