/* eslint-env browser */

/* Сохраненные данные о пути сохранения РК на хосте */
var bh_objects = {};

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
  $("#img_backup_ald_load").hide();
  $("#img_backup_host_load").hide();
  $("#img_backup_settings").hide();
  backupAldList();
  $("#cbox_host_backup").change(function() {
    let selectedHost = $(this).children("option:selected").val();
    let host = $(this).children("option:selected").text();
    let btnCreateHostBackup = $("#btn_create_host_backup");
    let imgSettings = $("#img_backup_settings");
    btnCreateHostBackup.attr("disabled", selectedHost == 0);
    selectedHost == 0 ? imgSettings.hide() : imgSettings.show();
    let divHostBackups = $("#div_host_backups");
    divHostBackups.empty();
    if(selectedHost == 0) {
      btnCreateHostBackup.attr("title", "");
    } else {
      loadHostBackup(host);
    }
  });
});

// Подгружаем список созданых бэкапов хоста
function loadHostBackup(host) {
  let cboxBackupHost = $("#cbox_host_backup");
  let btnCreateHostBackup = $("#btn_create_host_backup");
  let divHostBackups = $("#div_host_backups");
  let imgSettings = $("#img_backup_settings");

  btnCreateHostBackup.attr("title", "РК хоста будет создана в " + bh_objects[host]);
  btnCreateHostBackup.tooltip();

  sendAjaxRequest("/backup", "action=get_bh_list&host=" + host, function(data) {
    let count = 1;
    let listOfNames = JSON.parse(data);
    if(listOfNames.state === "done") {  // Только если все прошло успешно
      let mass = [];
      for(let i in listOfNames.data) {
        mass.push(listOfNames.data[i].datetime);
      }
      for(let i in listOfNames.ftp) {
        mass.push(listOfNames.ftp[i].datetime + "-");
      }
      mass.sort();
      for(let i in mass) {
        if(mass[i][mass[i].length - 1] === "-") {
          let textDate = mass[i].substr(0, mass[i].length - 1);
          addLine((parseInt(i) + 1) + ": " + "[ftp]" + textDate, "ftp_bh_list");
        } else {
          addLine((parseInt(i) + 1) + ": " + mass[i], "bh_list");
        }
      }
    }
  });
  // Добавляем очередную строку с наименованием бэкапа
  function addLine(name, class_name) {
    var divElement = $('<div>', {text: name, class: class_name});
    var imgElement = $('<img>', {src: "images/install.png", class: "bh_list_install"});
    var remElement = $('<img>', {src: "images/delete.png", class: "bh_list_install"});
    imgElement.click(function(){ // Восстановление бэкапа хоста
      if(confirm("Загрузить резервную копию?")) {
        btnCreateHostBackup.attr("disabled", true);
        cboxBackupHost.attr("disabled", true);
        sendAjaxRequest("/backup", "action=bh_load&host=" + host + "&datetime=" + $(this).parent().text().split(": ")[1], function(data) {
          var listOfNames = JSON.parse(data);
          if(listOfNames.state === "error_no_curl") {
            if(confirm("На хосте нет пакета CURL, установить его?")) {
              $("#img_backup_host_load").show();
              installCurlOnHost(host);
            }
          } else if (listOfNames.state === "error_ftp_denied") {
            jqcAlert("К FTP нет доступа");
            btnCreateHostBackup.attr("disabled", false);
            cboxBackupHost.attr("disabled", false);
          } else if (listOfNames.state === "error_ftp_pass") {
            jqcAlert("Логин или пароль заданы неверно");
            btnCreateHostBackup.attr("disabled", false);
            cboxBackupHost.attr("disabled", false);
          } else {
            cboxBackupHost.attr("disabled", false);
            cboxBackupHost.val(0);
            divHostBackups.empty();
            imgSettings.hide();
          }
          
        });
      }
    });
    remElement.click(function(){  // Удаление бэкапа хоста
      if(confirm("Удалить резервную копию?")) {
        btnCreateHostBackup.attr("disabled", true);
        cboxBackupHost.attr("disabled", true);
        sendAjaxRequest("/backup", "action=bh_remove&host=" + host + "&datetime=" + $(this).parent().text().split(": ")[1], function(data) {
          var listOfNames = JSON.parse(data);
          if(listOfNames.state === "error_no_curl") {
            if(confirm("На хосте нет пакета CURL, установить его?")) {
              $("#img_backup_host_load").show();
              installCurlOnHost(host);
            }
          } else if (listOfNames.state === "error_ftp_denied") {
            jqcAlert("К FTP нет доступа");
            btnCreateHostBackup.attr("disabled", false);
            cboxBackupHost.attr("disabled", false);
          } else if (listOfNames.state === "error_ftp_pass") {
            jqcAlert("Логин или пароль заданы неверно");
            btnCreateHostBackup.attr("disabled", false);
            cboxBackupHost.attr("disabled", false);
          }
          else {
            cboxBackupHost.attr("disabled", false);
            cboxBackupHost.val(0);
            divHostBackups.empty();
            imgSettings.hide();
          }
          
        });
      }
    });
    divElement.append(remElement);
    divElement.append(imgElement);
    divHostBackups.append(divElement);
  }
}

/* Запрос списка всех доступных РК ALD */
function backupAldList() {
  "use strict";
  let imgLoader = $("#img_backup_loader");
  let imgLoaderStart = $("#img_backup_loader_start");
  imgLoaderStart.hide();
  imgLoader.show();
  $("#backup_main_table").show().css( "display", "none" );
  let tableObj = $("#tbl_backups");
  let cboxBackupHost = $("#cbox_host_backup");
  let btnCreateALDBackup = $("#btn_create_backup");
  let btnCreateHostBackup = $("#btn_create_host_backup");
  tableObj.empty();
  cboxBackupHost.empty();
  sendAjaxRequest("/backup", "action=get_list", function(data) {
    $("#backup_main_table").show().css( "display", "block" );
    let count = 1;
    let listOfNames = JSON.parse(data);
    if(listOfNames.is_timeout === "true") {
      jqcAlert("Процесс завершен по таймауту");
    }
    btnCreateALDBackup.attr("title", "Резервная копия ALD будет создана в " + listOfNames.ald_backup_path);
    btnCreateALDBackup.tooltip();
    imgLoader.hide();
    imgLoaderStart.show();
    for(let i in listOfNames.data) {
      addRow(count++, listOfNames.data[i].date, listOfNames.data[i].time, listOfNames.data[i].size, listOfNames.data[i].is_load, listOfNames.data[i].is_loading);
    }
    btnCreateHostBackup.prop('disabled', true);
    btnCreateHostBackup.attr("title", "");
    if(listOfNames.hosts_list.length === 0) {
      addOption(0, "Нет хостов");
      cboxBackupHost.prop("disabled", true);
    }
    else {
      let Count_i = 1;
      addOption(0, "Выберите хост");
      for(let i in listOfNames.hosts_list) {
        addOption(Count_i++, listOfNames.hosts_list[i].host);
        bh_objects[listOfNames.hosts_list[i].host] = (listOfNames.hosts_list[i].server_type === '0' ? 'local://' : 'ftp://') + 
        listOfNames.hosts_list[i].server;
      }
    }
    /* Добавляем имя хоста в выпадающий список */
    function addOption(value, text) {
      cboxBackupHost.append($("<option>", {value: value, text:text }));
    }
    /* Добавление строки в таблицу */
    function addRow(count, date, time, size, is_load, is_loading) {
      let trObj = document.createElement("TR");
      if(is_load === "true") {
        trObj.setAttribute("class", (is_load === "true") ? "b_insert" : "");
      }
      createTD(count);
      createTD(formatDateShort(date));
      createTD(time);
      createTD(String(Number(size) / 1000) + "Kb");
      createTDImage(is_loading === "true" ? "images/loader.gif" : "images/install.png", "installBackup(\"" + date + "_" + time + "\", this)");
      createTDImage("images/delete.png", "removeBackup(\"" + date + "_" + time + "\")");

      tableObj.append(trObj);
      /* Добавление элемента в строку */
      function createTD(text) {
        $('<td/>', {align: "center", text: text}).appendTo(trObj);
      }
      /* Добавление элемента с картинкой в строку */
      function createTDImage(img_name, func) {
        let tdObj = $('<td/>', {align: "center", append: $('<img>', {
          src: img_name, width: 24, height: 24, on: {
          click: function(event) {eval(func);}
        } }),}).appendTo(trObj);
      }
    }
  });
}
/* Создание новой резервной копии ALD */
function backupAldStart() {
  let imgBackupAldLoad = $("#img_backup_ald_load");
  var btnCreateBackup = $("#btn_create_backup");
  var tx_button = btnCreateBackup.html(); 
  imgBackupAldLoad.show();
  btnCreateBackup.contents().first().remove();
  
  btnCreateBackup.prop("disabled", true);
  sendAjaxRequest("/backup", "action=save&file=new", function(data) {
    backupAldList();
    imgBackupAldLoad.hide();
    btnCreateBackup.prop("disabled", false);
    let listOfNames = JSON.parse(data);
    if(listOfNames.state != "done") {
      jqcAlert(listOfNames.state);
    }
    btnCreateBackup.html(tx_button);
  });
}

/* Восстановление ALD из резервной копии */
function installBackup(dtime, object) {
  if(confirm("Загрузить резервную копию?")) {
    var imageObject = $(object);
    imageObject.attr("src", "images/loader.gif");
    sendAjaxRequest("/backup", "action=load&dtime=" + dtime, function(data) {
      let listOfNames = JSON.parse(data);
      if(listOfNames.state != "done") {
        jqcAlert(listOfNames.state);
      }
      backupAldList();
    });
  }
}

/* Удаление ALD резервной копии */
function removeBackup(dtime) {
  if(confirm("Удалить резервную копию?")) {
    sendAjaxRequest("/backup", "action=remove&dtime=" + dtime, function(data) {
      backupAldList();
    });
  }
}
/* Создание резервной копии хоста */
function backupHostStart() {
    

    
  var imgBackupHostLoad = $("#img_backup_host_load");
  var btnCreateHostBackup = $("#btn_create_host_backup");
  var tx_button = btnCreateHostBackup.html();
  imgBackupHostLoad.show();
  var cboxHostBackup = $("#cbox_host_backup");
  var host = $("#cbox_host_backup :selected").text();
  var divHostBackups = $("#div_host_backups");
  var imgSettings = $("#img_backup_settings");
  
  sendAjaxRequest("/action", "resourcebyhost=test_ssh_connect&resourcehost=" + host, function(data) {
    var listOfNames = JSON.parse(data);
    if(listOfNames.data.update != "OK") {
      jqcAlert("<center>Отсутствует соединение SSH</center>");
      btnCreateHostBackup.html(tx_button);
      return 1;  
    }
    else {
        

  cboxHostBackup.prop("disabled", true);
  btnCreateHostBackup.prop("disabled", true);
  btnCreateHostBackup.contents().first().remove();
  
  sendAjaxRequest("/backup", "action=backup_host&&host=" + host, function(data) {
    let listOfNames = JSON.parse(data);
    imgBackupHostLoad.hide();
    cboxHostBackup.prop("disabled", false);
    if(listOfNames.state === "done") {
      divHostBackups.empty();
      loadHostBackup(host);
      btnCreateHostBackup.prop("disabled", false);
    } else if(listOfNames.state === "error") {
      divHostBackups.empty();
      jqcAlert("Ошибка при создании резервной копии " + host + "\n" +listOfNames["error_text"]);
      cboxHostBackup.val(0);
      imgSettings.hide();
    } else if(listOfNames.state === "abort") {
      divHostBackups.empty();
      jqcAlert("Невозможно подключиться к хосту " + host);
      cboxHostBackup.val(0);
      imgSettings.hide();
    } else if (listOfNames.state === "error_no_curl") {
      if(confirm("На хосте нет пакета CURL, установить его?")) {
        imgBackupHostLoad.show();
        installCurlOnHost(host);
      }
    } else if (listOfNames.state === "error_ftp_denied") {
      jqcAlert("К FTP нет доступа");
      cboxHostBackup.prop("disabled", false);
      btnCreateHostBackup.prop("disabled", false);
    } else if (listOfNames.state === "error_ftp_pass") {
      jqcAlert("Логин или пароль заданы неверно");
      cboxHostBackup.prop("disabled", false);
      btnCreateHostBackup.prop("disabled", false);
    }
    
     btnCreateHostBackup.html(tx_button);
    
  });
        
    }
  }); 
  
}
//----- Установка curl на хост -----
function installCurlOnHost(host) {
  sendAjaxRequest("/backup", "action=install_curl&host=" + host, function(data) {
    var listOfNames = JSON.parse(data);
    if(listOfNames["state"] === "done") {
      backupHostStart();
    } else {
      jqcAlert("Ошибка при установки CURL на хост");
      $("#cbox_host_backup").val(0);
      $("#img_backup_settings").hide();
      $("#img_backup_host_load").hide();
    }
  });
}

//----- Отображение настроек РК для хоста -----
function showBackupSettings() {
  var host = $("#cbox_host_backup :selected").text();
  var btnAddFolder = $("#bhs_btn_add_folder");
  var btnCancel = $("#bhs_cancel");
  var btnAccept = $("#bhs_accept");
 
    sendAjaxRequest("/action", "resourcebyhost=test_ssh_connect&resourcehost=" + host, function(data) {
    var listOfNames = JSON.parse(data);
    if(listOfNames.data.update != "OK") {
      jqcAlert("<center>Отсутствует соединение SSH</center>");
      return 1;  
    }
    else{
    
        btnAddFolder.off('click');
  btnCancel.off('click');
  btnAccept.off('click');
  var tbDirs = $("#bhs_tb_dirs");
  tbDirs.empty();
  let selBHSServerType = $("#bhs_server_type");
  let divBHSAuth = $("#bhs_div_auth");
  let inputBHSServerPath = $("#bhs_server_path");
  let saveType = '';
  let savePath = '';
  let saveLogin = '';
  let savePassword = '';
  selBHSServerType.change(function(){
    if(selBHSServerType.val() === saveType) {
      $('#bhs_login').val(saveLogin);
      $('#bhs_password').val(savePassword);
      inputBHSServerPath.val(savePath);
    } else {
      inputBHSServerPath.val("");
      $('#bhs_login').val("");
      $('#bhs_password').val("");
    }
    if(selBHSServerType.val() === "0") {
      divBHSAuth.slideUp(500);
      inputBHSServerPath.attr("placeholder", "Путь к хранилищу");
    } else {
      divBHSAuth.slideDown(500);
      inputBHSServerPath.attr("placeholder", "Адрес сервера");
    }
  });
  // Получем настройки для выбранного хоста
  sendAjaxRequest("/backup", "get=host_settings&host=" + host, function(data) {
    let listOfNames = JSON.parse(data);
    if(listOfNames.state === "error") {  // Только если все прошло плохо
      jqcAlert("Не удалось считать настройки из БД");
    } else if(listOfNames.host != host){
      jqcAlert("Пришла информация не для выбранного хоста");
    } else {
      $('#bhs_server_type option[value="' + listOfNames.server_type + '"]').prop('selected', true);
      if(selBHSServerType.val() === "0") {
        divBHSAuth.hide();
        inputBHSServerPath.attr("placeholder", "Путь к хранилищу");
      } else {
        divBHSAuth.show();
        inputBHSServerPath.attr("placeholder", "Адрес сервера");
      }
      $('#bhs_server_path').val(listOfNames.server);
      $('#bhs_login').val(listOfNames.login);
      $('#bhs_password').val(listOfNames.password);
      saveType = selBHSServerType.val();
      savePath = listOfNames.server;
      saveLogin = listOfNames.login;
      savePassword = listOfNames.password;

      var listDirs = listOfNames.dirs.split(';');
      for(var i in listDirs) {
        if(listDirs[i].length > 0) {
          if(listDirs[i][0] === '-') {
            addRow(listDirs[i].substr(1, listDirs[i].length), false);
          } else {
            addRow(listDirs[i], true);
          }
        }
      }
    }
  });
  // Добавляет строку с папками для РК хоста
  function addRow(name, isDir) {
    var trObj = $('<tr>');
    var tdCountObj = $('<td/>', {align: "center"});
    var tdName = $('<td/>', {align: "left", text: name, class: "bhs_dir_name"});
    var tdClose = $('<td/>', {align: "center"});
    tdClose.append($('<img>', {src: "images/delete.png", width: "16px", height: "16px", class: "btn_img_bhs", on: {click: function(event){ removeControlDir(this);}}}));
    tdCountObj.append($('<img>', {src: isDir ? "images/Folder-icon.png" : "images/state_write.png", width: "24px", height: "24px"}));
    trObj.append(tdCountObj);
    trObj.append(tdName);
    trObj.append(tdClose);
    tbDirs.append(trObj);
  }
  // Удаление строки
  function removeControlDir(object) {
    $(object).closest("tr").remove();
  }
  // Действия по клику на кнопку добавления папки
  btnAddFolder.click(function() {
    showHFWindow(host, function(name, isDir) {
      addNewDirectory(name, isDir);
      // showControlDialogFilesystem(tbSet, name);
    });
  });
  // Действие при нажатии кнопки ОТМЕНА
  btnCancel.click(function() {
    $("#backup_host_settings").dialog('close');
  });
  // Действие при нажатии кнопки ПРИМЕНИТЬ
  btnAccept.click(function() {
    var hostSettings = {};
    var dirsSetArr = [];
    hostSettings.server_type = $('#bhs_server_type :selected').val();
    hostSettings.server = $('#bhs_server_path').val();
    hostSettings.login = $('#bhs_login').val();
    hostSettings.password = $('#bhs_password').val();
    if(hostSettings.server_type === '0' && hostSettings.server.charAt(0) != '/') {
      jqcAlert("Локальный путь должен начинаться с /");
      return;
    }
    $('#bhs_tb_dirs tr').each(function() {
      var setCount = 0;
      var dirSetObj = {};
      var isDir = false;
      $(this).find('td').each(function(){
        if(setCount === 0) {
          isDir = ($(this).find('img').attr("src") === "images/Folder-icon.png");
        } else if(setCount === 1) {
          dirSetObj.path = $(this).text();
          dirSetObj.type = isDir ? "dir" : "file";
          dirsSetArr.push(dirSetObj);
        } 
        setCount++;
      });
    });
    hostSettings.dirs = dirsSetArr;
    let btnCreateHostBackup = $("#btn_create_host_backup");
    var str = JSON.stringify(hostSettings, "");
    // Применяем настройки для выбранного хоста
    sendAjaxRequest("/backup", "set=host_settings&host=" + host + "&settings=" + str, function(data) {
      var listOfNames = JSON.parse(data);
      if(listOfNames.state === "error") {  // Только если все прошло плохо
        jqcAlert("Не удалось записать настройки в БД");
      } else {
        $("#backup_host_settings").dialog('close');
        bh_objects[host] = (hostSettings.server_type === '0' ? 'local://' : 'ftp://') + hostSettings.server;
        btnCreateHostBackup.attr("title", "Технические данные РК хоста будут созданы в " + bh_objects[host]);
        btnCreateHostBackup.tooltip();
      }
    });
  });
  // Добавляет очередной каталог 
  function addNewDirectory(name, isDir) {
    var isCorrect = true;
    tbDirs.children('tr').each(function() {
      var count = 0;
      $(this).find('td').each(function(){
        if(count === 1 && $(this).text() === name) {
          isCorrect = false;
        }
        count++;
      });
    });
    if(isCorrect === true) {
      addRow(name, isDir);
    } else { 
      jqcAlert("Элемент для проверки уже выбран");
    }
  }

  $("#backup_host_settings").dialog({
    width: '400px',
    height: 'auto',
    modal: true
  });
        
    }
     
  });   
      
}
