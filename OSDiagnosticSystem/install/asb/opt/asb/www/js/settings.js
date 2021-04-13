/* Действия при загрузке этого js */
$(document).ready(function() { 
    let titleDict = {};
    titleDict["action"] = "Настройки действий ПО";
    titleDict["antivirus"] = "Настройки антивируса";
    titleDict["backup"] = "Настройки РК";
    titleDict["docroot"] = "Настройки клиентского ПО";
    titleDict["integrity_control"] = "Настройки КЦ";
    titleDict["journals"] = "Настройки журналирования";
    titleDict["listener"] = "Настройки связи клиент-сервер";
    titleDict["logging"] = "Настройки логирования";
    titleDict["login"] = "Настройки единой учетной записи администратора ПО АСБ";
    titleDict["pipe"] = "Настройки внутреннего доступа";
    titleDict["security"] = "Настройки тестирования";
    titleDict["sessions"] = "Настройки сессий";
    titleDict["sql"] = "Настройки БД";
    titleDict["users"] = "Настройки управления пользователями";

    let nameDict = {};
    nameDict["none"] = "Пустышка";
    nameDict["drwebInstallPath"] = "Путь к установочным файлам Dr.Web";
    nameDict["drwebKey"] = "Путь к файлу лицензионных ключей Dr.Web";
    nameDict["drwebUpdateUrl"] = "Путь к обновлениям Dr.Web";
    nameDict["kasperskyAutoinstall"] = "Путь к файлу первоначальной настройки Kaspersky";
    nameDict["kasperskyInstallPath"] = "Путь к установочным файлам Kaspersky";
    nameDict["kasperskyKey"] = "Путь к файлу лицензионных ключей Kaspersky";
    nameDict["kasperskyUpdateUrl"] = "Путь к обновлениям Kaspersky";
    nameDict["backupPath"] = "Путь к директории с РК ALD";
    nameDict["backupPathJournals"] = "Путь к директории хранения журналов";
    nameDict["hostBackupServer"] = "Путь к хранилищу ссылок на локальные РК";
    nameDict["cacheSize"] = "Размер кэша (байт)";
    nameDict["cacheTime"] = "Время хранения кэша (мс)";
    nameDict["encoding"] = "Кодировка";
    nameDict["maxAge"] = "Максимальный срок действия cookie (мс)";
    nameDict["maxCachedFileSize"] = "Размера файла кэширования (байт)";
    nameDict["path"] = "Путь к фронтэнду";
    nameDict["tmpPath"] = "Директория для обработки логов";
    nameDict["cleanupInterval"] = "Время до сброса соединения (мс)";
    nameDict["maxMultiPartSize"] = "Максимальный размер составного запроса (байт)";
    nameDict["maxRequestSize"] = "Максимальный размер запроса (байт)";
    nameDict["maxThreads"] = "Максимальное количество потоков";
    nameDict["minThreads"] = "Минимальное количество потоков";
    nameDict["port"] = "Номер порта";
    nameDict["readTimeout"] = "Ожидание полного HTTP запроса (мс)";
    nameDict["bufferSize"] = "Размер буфера (байт)";
    nameDict["fileName"] = "Файл лога";
    nameDict["maxBackups"] = "Максимальное количество РК";
    nameDict["maxSize"] = "Размер лога (байт)";
    nameDict["minLevel"] = "Минимальный уровень логирования";
    nameDict["msgFormat"] = "Формат лога";
    nameDict["timestampFormat"] = "Формат времени лога";
    nameDict["password"] = "Пароль";
    nameDict["username"] = "Имя пользователя (учетная запись с root-полномочиями)";
    nameDict["pfilename"] = "Файл внутренних команд";
    nameDict["cookieComment"] = "Коментарий cookie \
    (устанавливает куки, которые используются для идентификации\
    неавторизованного пользователя при комментировании)";
    nameDict["cookieName"] = "Имя cookie";
    nameDict["cookiePath"] = "Корневой путь cookie";
    nameDict["expirationTime"] = "Время сброса cookie (мс)";
    nameDict["databasename"] = "Файл БД";
    nameDict["WebServerHostIP"] = "Адрес удаленного веб-сервера для КП СГП";
    nameDict["installedSoftwarePath"] = "Путь к исполняемым файлам ограниченного доступа";
    nameDict["userOutputPath"] = "Каталог для экспорта информации о пользователях";
    nameDict["ald_server"] = "ALD сервер (основной используемый контроллер домена)";

    function getKeyByValue(object, value) {
        return Object.keys(object).find(key => object[key] === value);
      }
    $("#settings_reboot").tooltip();
    $("#img_settings").click(function() {
        var tbSettings = $("#tb_settings");
        $("#btn_settings_accept").off('click');
        $("#btn_settings_close").off('click');
        tbSettings.empty();
        sendAjaxRequest("/settings", "get=list", function(data) {
            var listOfNames = JSON.parse(data);
            let titleObject;
            for(i in listOfNames.data) {
                if(listOfNames.data[i].type === "title") {
                    titleObject = addTitle(listOfNames.data[i].name);
                } else if(listOfNames.data[i].type === "td") {
                    addRow(listOfNames.data[i].name, listOfNames.data[i].val, titleObject);
                }
            }
        });

        $('#dialog_settings').dialog({
            width: '800',
            height: 'auto',
            modal: true
        });
        /* Добавляет заголовок */
        function addTitle(title) {
            let trObj = $('<tr/>');
            let tdObj = $('<td/>', {align: "center", text: titleDict[title], colspan: 3, class: "td_settings_title"});
            trObj.append(tdObj);
            tbSettings.append(trObj);
            return trObj;
        }
        /* Добавляет строку */
        function addRow(name, value, title) {
            let trObj = $('<tr/>');
            let tdObjName = $('<td/>', {align: "center", text: name, class: "td_settings_name"});
            let tdObjVal = $('<td/>', {align: "center", class: "td_settings_val"});
            if(name === 'password') {   // Для пароля добавим элемент input
                tdObjVal.append($('<input/>', {type: "password", value: value}));
            } else {
                tdObjVal.text(value);
            }
            let tdObjDescript = $('<td/>', {align: "left", text: nameDict[name], class: "td_settings_descript"});
            tdObjVal.attr("contenteditable", true);
            trObj.append(tdObjDescript);
            trObj.append(tdObjVal);
            trObj.append(tdObjName);

            // tdObjName.hide();
            tbSettings.append(trObj);
            if(name === "none") {
                trObj.hide();
                title.hide();
            }
        }
        /* Действия по кнопки закрыть */
        $("#btn_settings_close").click(function(){
            $('#dialog_settings').dialog('close');
        });
        /* Действия по кнопки применить */
        $("#btn_settings_accept").click(function() {
            var isStart = false;
            var returnData = "{ ";
            $('#settings_table tr').each(function() {
                var tdTitle = $(this).find('.td_settings_title').text();
                if(tdTitle === "") {    // Если строка пустая
                    var tdName = $(this).find('.td_settings_name').text();
                    var orig_val;

                    let tdValObj = $(this).find('.td_settings_val');
                    let tdVal = tdValObj.text();
                    if(tdName === 'password') {
                        tdVal = tdValObj.find('input').val();
                    } 
                    returnData += ' {"' + tdName + '":"' + tdVal + '"},';
                } else {
                    if(isStart === true) {
                        returnData = returnData.slice(0, -1);
                        returnData += "],";
                    }
                    returnData += '"' + getKeyByValue(titleDict, tdTitle) + '": [';
                    isStart = true;
                }
            });
            returnData = returnData.slice(0, -1);
            returnData += "] }";

            sendAjaxRequest("/settings", "set=list&list=" + returnData, function(sub_data) {
                var subListOfNames = JSON.parse(sub_data);
                if(subListOfNames.status === "error") {
                    jqcAlert("Настройки не могут быть применены");
                } else {
                    if(confirm("Приняты новые настройки, перезапустить сервер?")) {
                        settings_reboot();
                    }
                }
            });
        });
    });
});

//----- Перезапуск сервера -----
function settings_reboot() {
    let TimeoutCounter = 15;
    let RebootMessage = $("<div></div>").html("Перезагрузка через " + TimeoutCounter + " секунд").dialog({
        dialogClass: "jqc_alert",
        title: "Перезагрузка сервиса",
        resizable: false,
        open: function() {                         // open event handler
            $(this)                                // the element being dialogged
                .parent()                          // get the dialog widget element
                .find(".ui-dialog-titlebar-close") // find the close button for this dialog
                .hide();                           // hide it
        },
        modal: true
      });
      
    sendAjaxRequest("/settings", "action=reboot");
    var timerId = setInterval(function() {
        RebootMessage.html("Перезагрузка через " + --TimeoutCounter + " секунд");
        if(TimeoutCounter === 0) {
            clearInterval(timerId);
            RebootMessage.dialog('close');
            sendAjaxRequest("/settings", "get=list");
        }
    }, 1000);
}
