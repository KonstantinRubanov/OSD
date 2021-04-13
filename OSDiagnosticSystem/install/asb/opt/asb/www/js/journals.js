/* eslint-env browser */


function formatDate(date) {
  var d = new Date(date),
  month = '' + (d.getMonth() + 1),
  day = '' + d.getDate(),
  year = d.getFullYear();

  var hours   = d.getHours();
  var minutes = d.getMinutes();
  var seconds = d.getSeconds();

  if (month.length < 2)   month = '0' + month;
  if (day.length < 2)     day = '0' + day;
  if (hours   < 10) hours = '0' + hours;
  if (minutes < 10) minutes = '0' + minutes;
  if (seconds < 10) seconds = '0' + seconds;

  return [day, month, year].join('.') + " " +  [hours, minutes, seconds].join(':');
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
  //journalGetList(); 
  
});

/* Запрос списка всех доступных журналов регистрации и учета */
function journalGetList() {
    "use strict";
    var imgLoader = $("#img_jrnl_loader");
    var imgLoaderStart = $("#img_jrnl_loader_start");
    imgLoaderStart.hide();
    imgLoader.show();
    $("#journals_main_table").show().css( "display", "none" );
    var tableObj = document.getElementById("tbl_journals");
    while (tableObj.firstChild) {
        tableObj.removeChild(tableObj.firstChild);
    }
    
    sendAjaxRequest("/journals", "get=data", function(data) {
      $("#journals_main_table").show().css( "display", "block" );
      imgLoader.hide();
      imgLoaderStart.show();
      var count = 1;
      var listOfNames = JSON.parse(data);
      for (var i in listOfNames.data) {
        if(listOfNames.data[i].type == "archive") {
            addRow(count++, listOfNames.data[i].start_date, listOfNames.data[i].stop_date,
              listOfNames.data[i].create_date_time, listOfNames.data[i].link);
        }
        else {
            var StartDate = document.getElementById("log_start_date");
            StartDate.setAttribute("min", listOfNames.data[i].start_date);
            StartDate.setAttribute("max", listOfNames.data[i].stop_date);
            StartDate.setAttribute("value", listOfNames.data[i].start_date);
            var StopDate = document.getElementById("log_stop_date");
            StopDate.setAttribute("min", listOfNames.data[i].start_date);
            StopDate.setAttribute("max", listOfNames.data[i].stop_date);
            StopDate.setAttribute("value", listOfNames.data[i].stop_date);
        }
      }
      /* Добавление строки в таблицу */
      function addRow(count, date_start, date_stop, created_dt, link) {
        var matchChar = link.match(/^(.*?)[|·\/:]/);
        var link_file = link;
        if (matchChar) {
           var len_  = link.length;
           link_file = link.substring((matchChar[1].length + 1), len_); 
        } 
        var trObj = document.createElement("TR");
        createTD(count);
        createTD(formatDateShort(date_start) + '\n' + formatDateShort(date_stop));
        createTD(formatDate(created_dt));
        createTDImage("images/delete.png", "deleteJournal(\"" + link_file + "\")");
        createTDImage("images/print.png", "printJournal(\""   + link_file + "\", this)");
        createTDHrefImage("images/link.png", link);

        tableObj.appendChild(trObj);
        /* Добавление элемента в строку */
        function createTD(text) {
          $('<td/>', {align: "left", text: text}).appendTo(trObj);
        }
          /* Добавление элемента с картинкой в строку */
        function createTDImage(img_name, func) {
          $('<td/>', {align: "center", append: $('<img>', {
            src: img_name, width: 24, height: 24, on: {
            click: function(event) {eval(func);}
          } }),}).appendTo(trObj);
        }
          /* Добавление элемента с картинкой в строку */
        function createTDHrefImage(img_name, link) {
          var tdDownload = $('<td>', {align: "center",
            append: $('<a>', {
              href: link, append: $('<img>', {src: img_name, width: 24, height: 24})
            }),});
          tdDownload.attr("download", true);
          tdDownload.appendTo(trObj);      
          }
        }
    });
}
/* Создание архива журнала регистрации и учета */
function journalCreateArch() {
  var imgJournalCreateLoad = $("#img_journal_create_load");
  var btnJournalCreate = $("#journals_btn_create");
  var tx_button = btnJournalCreate.html();  
  btnJournalCreate.attr("disabled", true);
  imgJournalCreateLoad.show();
  btnJournalCreate.contents().first().remove();

  var logStart_date = document.getElementById("log_start_date");
  var logStop_date = document.getElementById("log_stop_date");
    sendAjaxRequest("/journals", "set=archive&start_date=" + logStart_date.value +
        "&stop_date=" + logStop_date.value, function(data) {
      var listOfNames = JSON.parse(data);
        journalGetList();
        imgJournalCreateLoad.hide();
        btnJournalCreate.attr("disabled", false);
      if(listOfNames["state"] === "error") {
        jqcAlert("Ошибка при создании архива журнала!");
      }      
      btnJournalCreate.html(tx_button);
      
    });
}

function deleteJournal(date_range) {
  if(confirm("Удалить архив " + date_range + "?")) {
    sendAjaxRequest("/journals", "archive=delete&date_range=" + date_range, function(data) {
      journalGetList();
      var listOfNames = JSON.parse(data);
      if(listOfNames.operation == 'delete_archive' && listOfNames.state == 'error') {
        jqcAlert("Удаление файла невозможно!");
      }
    });
  }
}
//----- Печать журнала -----
function printJournal(date_range, object) {
  var IsPrintDone_b = true;
  var Text_str = "";
  var imgJournalPrint = $(object);
  var jText = $("#journal_text");
  imgJournalPrint.attr("src", "images/loader.gif");
  sendAjaxRequest("/journals", "archive=print&date_range=" + date_range, function(data) {
    var ListOfNames = JSON.parse(data);
    if(ListOfNames.operation == 'print_log' && ListOfNames.state == 'error') {
      jqcAlert("Ошибка открытия журнала");
      imgJournalPrint.attr("src", "images/print.png");
    }
    else {
      for(var i in ListOfNames.data) {
        Text_str += ListOfNames.data[i].string + '<br>';
      }
      OutToPrinter(Text_str);
      // jText.html(Text_str);  // Пока оставлю для проверки ( в этой версии FireFox не работает)
      // $("#journal_text").printThis();
      imgJournalPrint.attr("src", "images/print.png");
    }
  });

  /* Печать текста через создание нового окна */
  function OutToPrinter(text_str) {
    var WinPrint = window.open('','','left=50,top=50,width=800,height=640,toolbar=0,scrollbars=1,status=0');
    WinPrint.document.write('<html><head></head><body>');
    WinPrint.document.write(text_str);
    WinPrint.document.write('</body></html>');
    WinPrint.document.close();
    WinPrint.focus();
    WinPrint.print();
    WinPrint.close();
 }
}
