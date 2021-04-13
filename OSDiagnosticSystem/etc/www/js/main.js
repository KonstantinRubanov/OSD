/* eslint-env browser */

//# sourceURL=js/tests.js
//# sourceURL=html/tests.html.

//Подгрузка страниц и контента через вкладки

/* Действия при загрузке этого js */
$(document).ready(function() { 
  $("#section_devices").load("../html/devices.html");
  $("#section_users").load("../html/users.html");
  $("#section_journals").load("../html/journals.html");
  $("#section_antivirus").load("../html/antivirus.html");
  $("#section_control").load("../html/control.html");
  $("#section_backup").load("../html/backup.html");
  $("#section_tests").load("../html/tests.html");
  $("#tab_settings").load("../html/settings.html");
  $("#tab_admin_journal").load("../html/admin-journal.html");
  $("#dialog_fs").load("../html/host_filesystem.html");  // Внес единый диалог работы с ФС хоста
} );

// $("#tab_devices").on("click",function(){
//         $("#section_devices").load("../html/devices.html");
// });
// $("#tab_users").on("click",function(){
//         $("#section_users").load("../html/users.html");
// });
// $("#tab_journals").on("click",function(){
//         $("#section_journals").load("../html/journals.html");
// });
// $("#tab_antivirus").on("click",function(){
//         $("#section_antivirus").load("../html/antivirus.html");
// });
// $("#tab_control").on("click",function(){
//         $("#section_control").load("../html/control.html");
// });
// $("#tab_backup").on("click",function(){
//         $("#section_backup").load("../html/backup.html");
// });
// $("#tab_tests").on("click",function(){
//         $("#section_tests").load("../html/tests.html");
// });

function ready() {
    $("#tab_devices").click();
}
document.addEventListener("DOMContentLoaded", ready);

/* Альтернативный способ выдачи запроса */
function sendAjaxRequest(url_str, data_str, func, timeout) {
  var ajaxTimeout = (timeout === "undefined" ? 0 : timeout);
  $.ajax({
    url: url_str,
    dataType: 'text',
    type: 'get',
    contentType: 'application/x-www-form-urlencoded',
    data: data_str,
    timeout: ajaxTimeout,
    success: function( data, textStatus, jQxhr ){
      func(data);
    },
    error: function( jqXhr, textStatus, errorThrown ) {
      console.log("sendAjaxRequest:: ", jqXhr, textStatus, errorThrown);
      if(jqXhr.status == 401) {
        location.href = '/';
      }
      else if(jqXhr.status === 0) {
        return;
      }
      else {
        alert('error: ' + jqXhr.status ? jqXhr.statusText : 'query failed');
      }
    }
  });
}

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
