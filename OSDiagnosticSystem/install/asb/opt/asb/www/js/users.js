
function callCloseJQueryDialog(){
 $(this).closest('.ui-dialog-content').dialog('close');
 $("#message_acl_dialog").text("");
}

$(document).ready(function() {
  $("#button_update_user_list").click(GetUsersALD);
  $("#btn_unlockAll").click(UnlockUsersAld);
  $('#btn_generate_password').click(CheckCgiHostAccess);
  $("#btn_list_passwords").click(ShowPasswordsList);
    
  $("#button_close_acl_dialog").click(callCloseJQueryDialog);
  $("#button_print_close").click(callCloseJQueryDialog);
  $("#button_print").click(PrintPasswordList);
    
  $("#button_lock").click(callLockUserFunction);
  $("#button_unlock").click(callLockUserFunction);
  $("#button_export_user_info").click(callExportUserInfoFunction);
  //диалог управления доступом пользователя к программам
  $("#button_acl_user_application").click(function(){
    $("#loader_acl_user_application").show();
    $("#button_acl_user_application").attr("disabled","true");
    GetHostList();      
   });
    //диалог управления генерацией паролей
    $("#button_next_first").click(callGeneratorSecondStepAction);
    $("#button_next_second").click(callGeneratorThirdStepAction);
    $("#button_next_third").click(CheckGeneratorFields);
    
    $("#button_previous_second").click(callGeneratorFirstStepAction);
    $("#button_previous_last").click(callGeneratorThirdStepAction);
    
    $("#button_close_generate_password_dialog").click(callCloseGeneratePasswordDialog);
    $("#print_password_list").click(PrintGeneratedPasswordList);
    
    //обработчики событий на чекбоксы алфавита в генераторе паролей.
    //если выбран хотя бы один, то разблокируется кнопка генерации
    callOnloadCheckAlphabethCbx();
});

function PrintPasswordList(){
    //Печать списка паролей по кнопке "Печать" в диалоге. Привязка кнопки и события при загрузке страницы
    $("#label_print_passwords").printThis();
}

var RowCount = 1;
var AllUsersInfoArray;
//заполнение таблицы пользователей
function GetUsersALD(){
    $("#tableUL").show();
    //снимаем выделения со всех строк в таблице, чтобы выделить выбранную
    $('#tableUL tr').each(function(){
      $(this).find('td').each(function(){
        $(this).toggleClass("on",false);
       })  
     }); 
    sendAjaxRequest("/users", "usersAld=userlist", function(data) {
       clearAll();
       AllUsersInfoArray = 0;
       var UserlistArray = JSON.parse(data);
       if (UserlistArray.operation === "list_users" && UserlistArray.state === "done"){
              AllUsersInfoArray = UserlistArray;
              var i;
              for (i = 0; i < UserlistArray.data.length; i++){
                addRow(UserlistArray.data[i]);
              }
              $("#EditUserInfo").hide();
        }
        else if (UserlistArray.operation === "list_users" && UserlistArray.state === "failed"){
          if (UserlistArray.user_list === "failed"){
              jqcAlert("Команда user-list возвращает пустой список пользователей.");
            }
          else if (UserlistArray.user_get === "failed"){
              var UserGetString = "Команда user-get не выполнена для пользователя " + UserlistArray.userName;
              jqcAlert(UserGetString);
            }
         }
         
         var table = $("#tableUL").DataTable();
    });
}

var userButtonIsCreated = false;
var currentUserName;
var lockUser = "false";

function ShowControlPanel(rowID) {//rowID = userName
    currentUserName = rowID;
    $("#EditUserHeader").html("Имя пользователя: " + currentUserName);  
    for (var i = 0; i < AllUsersInfoArray.data.length; i++){
        if(AllUsersInfoArray.data[i].UserName === rowID){
            createUserInfo(AllUsersInfoArray.data[i]);
            //alert(AllUsersInfoArray.data[i].UserName);
            if (AllUsersInfoArray.data[i].TemporaryBlocked == "Да"){
              $("#button_lock").attr("disabled","true");
              $("#button_unlock").removeAttr("disabled"); 
            }
            else if (AllUsersInfoArray.data[i].TemporaryBlocked == "Нет"){
              $("#button_unlock").attr("disabled","true");
              $("#button_lock").removeAttr("disabled");  
            }
        }
    }
    $("#EditUserInfo").show().transition(0.3);
}

function callLockUserFunction(){
    lockUser = ($(this).attr("id") === "button_lock") ? "true" : "false";
    var isLocked = window.confirm("Подтвердите действие");
//    var isLocked = $("#confirm_dialog").dialog({
//        title: "Подтвердите действие",
//        
//        closeOnEscape: true,
//        modal: true,
//        buttons: [
//            {
//                text: "Нет",
//                click: function() {
//                    $(this).dialog("close");
//                    return "false";
//                }
//            },
//            {
//                text: "Да",
//                click: function() {
//                    $(this).dialog("close");
//                    return "true";
//                }
//            }
//        ]
//    });
    if (isLocked == true){
      LockUser();
    }
}

function callExportUserInfoFunction(){
   if ($(this).attr("id") === "button_export_user_info"){
       ExportUserInfo();
   } 
}

function createUserInfo(userName) {
    if (AllUsersInfoArray != 0){
        var label_user = document.getElementById("label_user");
        label_user.innerHTML = "Ald аккаунт : " + userName.AldAccount + "<br>" + " <br>" +
                               "Тип ФС домашнего каталога : " + userName.HomeFileSystemType + "<br>" + " <br>" +
                               "Домашний каталог : " + userName.HomePath + "<br>" + " <br>" +
                               "Сервер домашнего каталога : " + userName.HomeServer + "<br>" + " <br>" +
                               "Дата последней смены пароля : " + userName.LastPasswordChangeDate + " <br>" + " <br>" +
                               "Окончание срока действия пароля : " + userName.PasswordExpirationDate + " <br>" + " <br>" +
                               "Политика пароля : " + userName.PasswordPolicy + " <br>" + " <br>" +
                               "Дата последней успешной аутентификации : " + userName.LastSuccesfullAuthentificationDate + " <br>" + " <br>" +
                               "Дата последней неуспешной аутентификации : " + userName.LastFailedAuthentificationDate + " <br>" + " <br>" +
                               "Количество неуспешных попыток ввода пароля : " + userName.NumberOfFailedPasswordEnters + " <br>" + " <br>" +
                               "Заблокирован за неудачные попытки входа : " + userName.BlockedForFailedEntries + " <br>" + " <br>" +
                               "Принципал Kerberos : " + userName.PrincipalKerberos + " <br>" + " <br>";
    }
}


function addRow(ArrayData) {
  var tableObj = document.getElementById("tableUserList_body");
  var trObj = document.createElement("TR");
    
  var userName = ArrayData.UserName;
  var NameField = ArrayData.FullUserName;
  var LoginField = ArrayData.UserName;
  var UIDField = ArrayData.UID;
  var TemporaryBlocked = ArrayData.TemporaryBlocked;
  var LastPasswordChangeDateField = ArrayData.LastPasswordChangeDate;
  var ChangePasswordStateField = (ArrayData.ChangePasswordState == "change_by_admin") ? "Изменен администратором" : "Изменен пользователем";
    
  createTD(trObj, RowCount, "IncrementRow", userName);
  createTD(trObj, NameField, "NameField", userName);
  createTD(trObj, LoginField, "LoginField", userName);
  createTD(trObj, UIDField, "UIDField", userName);
  var Locked = (TemporaryBlocked === "Нет") ? 'Не заблокирован' : 'Заблокирован';

  createTD(trObj, Locked, "TemporaryBlocked", userName);
  createTD(trObj, LastPasswordChangeDateField, "LastPasswordChangeDate", userName);
//  createTD(trObj, ChangePasswordStateField, "ChangePasswordState", userName);//кем изменен пароль
//    alert(ChangePasswordStateField);
// Группы
    var LocalGroups = '';
    for (var i in ArrayData.Groups.LocalGroup){
        LocalGroups +=  (LocalGroups.length)? (', ' + ArrayData.Groups.LocalGroup[i].GroupName):ArrayData.Groups.LocalGroup[i].GroupName;
    }
  createTD(trObj, LocalGroups, "LocalGroups", userName);
    var PrimaryGroup = ArrayData.Groups.PrimaryGroup[0].GroupName;
  createTD(trObj, PrimaryGroup, "PrimaryGroup", userName);
    var SecondaryGroup = ArrayData.Groups.SecondaryGroup[0].GroupName;
  createTD(trObj, SecondaryGroup, "SecondaryGroup", userName);

  tableObj.appendChild(trObj);
  RowCount++;
}

function createTD(tr, text, cellId, rowID) {
  var tdObj = document.createElement("TD");
  tr.appendChild(tdObj);
  tdObj.setAttribute("align", "center");
  tdObj.setAttribute("id", "#cellId");
  tdObj.addEventListener("click",function(){
         //очистить все выделения
           $('#tableUL tr').each(function(){
             $(this).find('td').each(function(){
                 $(this).toggleClass("on",false);
             })  
           });
        //выделить нужную строку
        $(this).closest("tr").each(function(){
            $(this).find('td').each(function(){
                 $(this).toggleClass("on",true);
             });  
        }); 
        ShowControlPanel(rowID);
    });
  tdObj.appendChild(document.createTextNode(text));
}

function clearAll() {
    RowCount = 1;
  var tableObj = document.getElementById("tableUserList_body");
  while (tableObj.firstChild) {
    tableObj.removeChild(tableObj.firstChild);
  }
}

function UnlockUsersAld() {
  var isLocked = confirm("Подтвердите действие");
  if (isLocked == false)
      return;
    sendAjaxRequest("/users", "usersAld=unlockAll", function(data) {
       var BlockUserArray = JSON.parse(data);
          if (BlockUserArray.operation === "lock_all_users" && BlockUserArray.state === "done"){
              GetUsersALD();
          }
    });
}

//Генерация паролей
function ShowGeneratePasswordDialog(){
   $("#div_dialog_generate_password").dialog({
                width: 500,
                height: 500,
                title: 'Генерация паролей',
                modal: true
              }); 
}

//Запросить список паролей и показать в диалоге
function ShowPasswordsList(){
    sendAjaxRequest("/users", "print_passwords=true", function(data) {
    var Array = JSON.parse(data);
          if (Array.operation === "passwords_list" && Array.state === "done"){
              var label_print_passwords = document.getElementById("label_print_passwords");
              //сначала почистить
              var TextContent = "";
              for (var i = 0; i < Array.data.length; i++){
                  TextContent = TextContent + Array.data[i].UserName + " : " + Array.data[i].Password + "<br>";
              }
              label_print_passwords.innerHTML = TextContent;
              $("#div_dialog_print_passwords").dialog({
                width: 400,
                height: 300,
                title: 'Пользователи и пароли',
                modal: true
              });
          }
    });
}

function LockUser(){
  sendAjaxRequest("/users", "lock_user=" + lockUser + "&&userName=" + currentUserName, function(data) {
    var BlockUserArray = JSON.parse(data);
    var locked_user_str;
    if (BlockUserArray.data[0].lock_user === "locked"){
      locked_user_str = "заблокирован";
     }
    else {
      locked_user_str = "разблокирован";
    }
    var command_status_str = "Пользователь " +  BlockUserArray.data[0].userName + " " + locked_user_str;
    if (BlockUserArray.operation === "lock_user_status" && BlockUserArray.state === "done"){
      jqcAlert(command_status_str);
//      if(locked_user_str == "заблокирован"){
//          $("#button_lock").attr("disabled","true");
//          $("#button_unlock").removeAttr("disabled");  
//      }
//      else if (locked_user_str == "разблокирован"){
//          $("#button_unlock").attr("disabled","true");
//          $("#button_lock").removeAttr("disabled");  
//       }
      GetUsersALD();
    }
    else {
      var lock_user_state_str = "Не удалось выполнить операцию. Пользователь уже " + locked_user_str;
      jqcAlert(lock_user_state_str);
      }
    });
}

function ExportUserInfo(){
  $("#export_loader").show();
  sendAjaxRequest("/users", "usersAld=export_user_output" + "&&user_name=" + currentUserName, function(data) {
    var Array = JSON.parse(data);
    var CommandState;
    if (Array.operation === "print_user_output"){
      CommandState = Array.state;
      $("#export_loader").hide();
      var u_tmp = 'Данные пользователя экспортированы в файл /tmp/user/' + currentUserName + '_user_output.txt'
      alert(u_tmp);
    }
  });
}

//Далее идут функции для работы с диалогом управления доступом к приложениям
var HostList = [];
var ACLFilesListToLock = [];
var ACLFilesListToUnlock = [];
var selectedHostName;

function ClearACLTables() {
  ACLFilesListToLock = [];
  ACLFilesListToUnlock = [];
  HostList = [];
  var tableObj = document.getElementById("table_host_body");
  while (tableObj.firstChild) {
    tableObj.removeChild(tableObj.firstChild);
  }
  tableObj = document.getElementById("table_unlocked_app_body");
  while (tableObj.firstChild) {
    tableObj.removeChild(tableObj.firstChild);
  }
  tableObj = document.getElementById("table_locked_app_body");
  while (tableObj.firstChild) {
    tableObj.removeChild(tableObj.firstChild);
  }
}

function GetHostList(){
   ClearACLTables(); 
    sendAjaxRequest("/users", "get=host-list", function(data) {
      var Array = JSON.parse(data);
      if (Array.operation === "get_hosts" && Array.state === "done"){
         for (var i = 0; i < Array.data.length; i++){
            HostList.push(Array.data[i].host);
          }
          var Title_dialog = "Доступ к приложениям: " + Array.path;
          $("#div_dialog_acl_user_application").dialog({
            title: Title_dialog,
            width: 820,
            height: 597,
            modal: true
           });
              
           ACLFillHostTable(HostList);
           $("#loader_acl_user_application").hide();
           $("#button_acl_user_application").removeAttr('disabled');
          }
    });
}

function ACLFillHostTable(array_data){
    var tableObj = document.getElementById("table_host_body");
    for (var i = 0; i < array_data.length; i++){
       var trObj = document.createElement("TR");
       ACLCreateRow("host-list", trObj, array_data[i], i);
       tableObj.appendChild(trObj);
    }
}

//заполняем таблицу с заблокированными и разблокированными приложениями для пользователя currentUserName
function ACLFillAppTables(array_data){
     var tableObj;
    for (var i = 0; i < array_data.length; i++){
      if (array_data[i].app_state == "unlocked"){
        tableObj = document.getElementById("table_unlocked_app_body");
       }
      else {
        tableObj = document.getElementById("table_locked_app_body");
      }
      var trObj = document.createElement("TR");
      ACLCreateRow("app-list", trObj, array_data[i].app_name, i);
      tableObj.appendChild(trObj);
    }
}

//добавить строку в таблицы
function ACLCreateRow(eventListenerType, tr, text, cellId) {
  var tdObj = document.createElement("TD");
  tdObj.setAttribute("align", "left");
  tdObj.setAttribute("valign", "bottom");
  tdObj.id = text;
    //если таблица хостовая, то добавляет на строку запрос на список приложенийи доступов для пользователя currentUserName
if (eventListenerType == "host-list"){
       tdObj.addEventListener("click",function(){
//           $("#message_acl_dialog").css("opacity","0");
           $("#message_acl_dialog").text("");
           //снимаем выделения со всех строк в таблице, чтобы выделить выбранную
           $('#table_host_body tr').each(function(){
             $(this).find('td').each(function(){
                 $(this).toggleClass("on",false);
             })  
           });
        //выделить нужную ячейку
       $(this).toggleClass("on");
       ///очистка таблиц и массивов(надо пределать)
       ACLFilesListToLock = [];
       ACLFilesListToUnlock = [];  
       tableObj = document.getElementById("table_unlocked_app_body");
       while (tableObj.firstChild) {
         tableObj.removeChild(tableObj.firstChild);
        }
       tableObj = document.getElementById("table_locked_app_body");
       while (tableObj.firstChild) {
         tableObj.removeChild(tableObj.firstChild);
       }
       $("#ll_locked_list").show();
       $("#ll_unlocked_list").show();
       ///
                 ///посмотрим, есть ли такой пользователь на хосте в качестве локального
      sendAjaxRequest("/users", "get=compare_with_local_users&host_name=" + text + "&user_name=" + currentUserName, function(data) {
          var Array = JSON.parse(data);
          if (Array.state == "done"){///если есть, то смотрим на приложения на этом хосте
            sendAjaxRequest("/users", "get=application-list&host_name=" + text + "&user_name=" + currentUserName, function(data) {
                var Array = JSON.parse(data);
                if (Array.operation == "get_applications"){
               //дублирование hide - переделать
               if(Array.state == "no_such_path"){
                 $("#message_acl_dialog").text("Указанный путь не существует.");
                 $("#ll_locked_list").hide();
                 $("#ll_unlocked_list").hide();
               }
               else if(Array.state == "no_exec_files"){
                 $("#message_acl_dialog").text("Исполняемых файлов не найдено.");
                 $("#ll_locked_list").hide();
                 $("#ll_unlocked_list").hide();
               }
               else if (Array.state == "empty_directory"){
                 $("#message_acl_dialog").text("Нет файлов.");
                 $("#ll_locked_list").hide();
                 $("#ll_unlocked_list").hide();  
               }
           }
       ACLFillAppTables(Array.data);
    });
          }
          else if (Array.state == "failed"){
             let WarningMess = "ACL: Пользователь " + currentUserName + " не входит в список локальных пользователей на хосте. Выберите другой хост.";
             jqcAlert(WarningMess); 
          }
          else {
             jqcAlert("ssh exception");  
          }
          $("#ll_locked_list").hide();
          $("#ll_unlocked_list").hide();
      });
                  
       selectedHostName = text;
    });//eventListener
}
    //если таблицы с приложениями, то добавляет на строку чекбокс
else if (eventListenerType == "app-list"){
       var check = document.createElement("input");
       check.className = "checkbox";
       check.type = "checkbox";
       check.align = "left";
       check.id = text;
       check.addEventListener("click", function() {
  });
       tdObj.appendChild(check);
    }
    tdObj.appendChild(document.createTextNode(text));
    tr.appendChild(tdObj);
}

//удалить строки c выделенными чекбоксами из таблицы
function deleteCheckedRows(table) {
  var checkboxes = table.getElementsByClassName("checkbox");
  var i = checkboxes.length;
  while (i--) {
      if (checkboxes[i].checked) {
          var tr = checkboxes[i].parentNode.parentNode;
             tr.parentElement.removeChild(tr);
      }
  }
}
//удалить все строки из таблицы
function deleteRows(table) {
    while(table.rows.length > 0){
        table.deleteRow(0);
    }
}

//удалить элемент из массива
function deleteArrayElement(Array, Element){
    for (var i = 0; i < Array.length; i++){
        if (Array[i] == Element){
          Array.splice(i,Element);
          break;
        }
    }
}

//заблокировать приложения, помеченные галочками
function callACLLockFiles(){
    var tableUnlockedObj = document.getElementById("table_unlocked_app_body");
    var tableLockedObj = document.getElementById("table_locked_app_body");
    var checkboxes = tableUnlockedObj.getElementsByClassName("checkbox");
    for (var index = 0; index < checkboxes.length; index++) {
      if (checkboxes[index].checked) { // положим в массив выбранный
        ACLFilesListToLock.push(checkboxes[index].id);
        deleteArrayElement(ACLFilesListToUnlock,checkboxes[index].id);
        $("#button_add").attr("disabled","true");
      }
    }
//    alert(ACLFilesListToLock.length);
    for (var i = 0; i < ACLFilesListToLock.length; i++){
        var Alert_str = "lock-access=true&host_name=" + selectedHostName +"&user_name=" + currentUserName + "&file_name=" + ACLFilesListToLock[i];
//        alert(Alert_str);
    sendAjaxRequest("/users", "lock-access=true&host_name=" + selectedHostName +"&user_name=" + currentUserName + "&file_name=" + ACLFilesListToLock[i], function(data) {
       var Array = JSON.parse(data);
       var trObj = document.createElement("TR");
       ACLCreateRow("app-list",trObj,Array.app_name,Array.app_name);
       tableLockedObj.appendChild(trObj);
       deleteCheckedRows(tableUnlockedObj);
       $("#button_add").removeAttr("disabled");
    });
    }
    ACLFilesListToLock = [];
}

//разблокировать приложения, помеченные галочками
function callACLUnlockFiles(){
    var tableUnlockedObj = document.getElementById("table_unlocked_app_body");
    var tableLockedObj = document.getElementById("table_locked_app_body");
    var checkboxes = tableLockedObj.getElementsByClassName("checkbox");
//    alert(checkboxes.length);
    for (var index = 0; index < checkboxes.length; index++) {
      if (checkboxes[index].checked) { // положим в массив выбранный
        ACLFilesListToUnlock.push(checkboxes[index].id);
        deleteArrayElement(ACLFilesListToLock,checkboxes[index].id);
        $("#button_remove").attr("disabled","true");
      }
    }
    
    for (var i = 0; i < ACLFilesListToUnlock.length; i++){
    sendAjaxRequest("/users", "lock-access=false&host_name=" + selectedHostName +"&user_name=" + currentUserName + "&file_name=" + ACLFilesListToUnlock[i], function(data) {
       var Array = JSON.parse(data);
       var trObj = document.createElement("TR");
       ACLCreateRow("app-list",trObj,Array.app_name,Array.app_name);
       tableUnlockedObj.appendChild(trObj);
       deleteCheckedRows(tableLockedObj);
       $("#button_remove").removeAttr("disabled");
    });
    }
    ACLFilesListToUnlock = [];
}

//диалог управления генерацией паролей
var SelectedUserNames = [];
var RemoteHostIPAddress;
let AlphabethCheckedNum_flag = 0;

function callOnloadCheckAlphabethCbx(){
    $(".invisible").click(function(){
        if (this.id == "cbx_set_flag")
            return;
        ($(this).is(":checked")) ? AlphabethCheckedNum_flag++ : AlphabethCheckedNum_flag--;
        if (AlphabethCheckedNum_flag > 0)
          $("#button_next_third").removeAttr("disabled");
        else
          $("#button_next_third").attr("disabled","true");
    });
}

function CheckCgiHostAccess(){//проверка, есть ли связь с удаленным хостом, есть ли generatorCgi
   sendAjaxRequest("/users", "check_cgi_host_access", function(data) {
       var Array = JSON.parse(data);
          if (Array.operation === "check_cgi_host_access" && Array.state === "done"){
              if (Array.host_state == "connected"){
                  RemoteHostIPAddress = Array.host_ip;
                  //проверка, что generatorCgi работает
                  var urlHost = "http://" + RemoteHostIPAddress + "/cgi-bin/generatorCgi";
                  $.ajax({ 
                    url: urlHost,
                    header: "Access-Control-Allow-Origin: *",
                    method: "POST",
                    data: {"cntPass": 1,"cntSymb": 8, "numAlph": 7},
       	            success: function(data) {
                      if(data.indexOf("ERR",0) === 0) {
                        jqcAlert(data);
                      }
                      else {
                        ShowGeneratePasswordDialog();  
                      }
                     },
                    error: function(data) {
                        jqcAlert("Генератор паролей отсутствует или не настроен");
                    }   
                   });
              }
              else {
                  jqcAlert("Нет связи с удаленным хостом");
              }
          }
    }); 
}


let ErrorTextMap = [];//ассив сообщений с ошибками установки пароля по кажд

function SetPasswords(PasswordMap){
  var Text;
  ErrorTextMap = [];
  $("#print_password_list").attr("disabled","true");
  callRecursiveRequest(PasswordMap, Text);
  //отсортируем таблицу
  var tableObj = document.getElementById("table_pass_body");
  let sortedRows = Array.from(tableObj.rows)
  .sort((rowA,rowB) => rowA.cells[0].innerHTML > rowB.cells[0].innerHTML ? 1 : -1);
    tableObj.tBodies[0].append(...sortedRows);
}

function callRecursiveRequest($Map, $MoreAboutErrorText){
    if (!$Map.length) return;

    let PasswordToSend = $Map[0].password;
    let NameToSend = $Map[0].name;
    
     sendAjaxRequest("/users", "change_password&user_name=" + NameToSend + "&new_password=" + encodeURIComponent(PasswordToSend), function(data) {
         
         var Array = JSON.parse(data);
         let Str = Array.user_name + " : " + Array.new_pswd;//заполняем строку таблицы с именем пльзователя и паролем
         var ShortPassState;
         var tableObj = document.getElementById("table_pass_body");
         var tdObj_state = document.createElement("TD"); //ячейка со статусом пароля
        //сообщения об ошибках
         if (Array.pswd_state == "done"){
           $MoreAboutErrorText = "Пароль успешно установлен.";
           ShortPassState = " Установлен";
           $("#message_generate_password_dialog").css("color","#BBB");
           tdObj_state.style.color = 'green';
           $("#print_password_list").removeAttr("disabled");
         }
         else {
           $("#message_generate_password_dialog").css("color","#b51212");
           tdObj_state.style.color = '#b51212';
           ShortPassState = " Не установлен";
           //ShortPassState="Сложность пароля не соответствует доменной политике.";
           if (Array.pswd_state == "not_enough_character_classes"){
             //ShortPassState = " Пароль не соответствует доменной политике.";
             ShortPassState="Сложность пароля не соответствует доменной политике.";
             $MoreAboutErrorText = "Сложность пароля не соответствует доменной политике.";
            }
           else if (Array.pswd_state == "cannot_reuse_password"){
             $MoreAboutErrorText = "Пароль не установлен: нельзя установить тот же пароль.";
            }
           else if (Array.pswd_state == "password_is_too_short"){
             $MoreAboutErrorText = "Пароль не установлен: пароль слишком короткий.";
           }
           else if (Array.pswd_state == "failed"){
             $MoreAboutErrorText = "Не удалось установить пароль.";
           }
           else if (Array.pswd_state == "rpc_handshake"){
             $MoreAboutErrorText = "Пароль не установлен: ошибка RPC протокола.";
           }  
         }
////           if (Array.user_name == "user3"){
//////               alert(Array.user_name);
////               Text = "Пароль не установлен: пароль слишком короткий.";
////           }
////           else {
////               Text = "Пароль установлен";
////           }
//        
        let UserPassState = new Object();
        UserPassState = {
               name: Array.user_name,
               state: $MoreAboutErrorText
           };
        ErrorTextMap.push(UserPassState);

        //пока заполнение в лоб
        var trObj = document.createElement("TR");
        //имя пользователя+пароль
        var tdObj = document.createElement("TD");
        tdObj.align = "center";
        tdObj.valign = "bottom";
        tdObj.id = "td_id";
        tdObj.appendChild(document.createTextNode(Str));
        //статус пароля
        tdObj_state.align = "center";
        tdObj_state.valign = "bottom";
        tdObj_state.id = Array.user_name;
        tdObj_state.addEventListener("click",function(){
          //очистить все выделения
          $('#table_pass_body tr').each(function(){
            $(this).find('td').each(function(){
              $(this).toggleClass("on",false);
            })  
          });
          //выделить нужную строку
          $(this).closest("tr").each(function(){
            $(this).find('td').each(function(){
              $(this).toggleClass("on",true);
            });  
          });
          for (var j = 0; j < ErrorTextMap.length; j++){
              if (ErrorTextMap[j].name == tdObj_state.id){
                  $("#message_generate_password_dialog").text(ErrorTextMap[j].state);
              }
          }
        });
           
        trObj.appendChild(tdObj);
        trObj.appendChild(tdObj_state);
        tableObj.appendChild(trObj);

        tdObj_state.appendChild(document.createTextNode(ShortPassState));
        $("#ll_pass_table").hide();
        callRecursiveRequest($Map.slice(1));
    });
}

function callGeneratorSecondStepAction(){
    $("#div_set_flag").hide();
    $("#div_generate_result").hide();
    if($("#cbx_set_flag").is(':checked')){//если пароли потребуется установить
      $("#div_set_password_properties").hide();
      $("#button_next_second").attr("disabled","true");
      RowCheckedCounter = 0;
      $("#div_set_users").css("display","grid");
      var tableObj = document.getElementById("table_user_body");
      sendAjaxRequest("/users", "usersAld=user_names", function(data) {
       var Array = JSON.parse(data);
       var NamesArray = [];
    
       for (var i = 0; i < Array.data.length; i++){
          NamesArray.push(Array.data[i].UserName); 
       }
       SelectedUserNames = [];
       GPDCreateTable(tableObj, NamesArray, true);
    });
        /*поправим нумерацию действий*/
        $("#numbering_pass_prop").text("3");
        $("#numbering_last_action").text("4");
    }
    else {
        /*поправим нумерацию действий*/
        $("#numbering_pass_prop").text("2");
        $("#numbering_last_action").text("3");
        
       callGeneratorThirdStepAction();
    }
}

function callGeneratorThirdStepAction(){
    $("#div_set_users").hide();
    $("#div_set_flag").hide();
    $("#div_generate_result").hide();
    $("#message_generate_password_dialog").text("");
    //сброс чекбоксов с алфавитом
    $("#cbx_latin").prop("checked",false);
    $("#cbx_numbers").prop("checked",false);
    $("#cbx_symbols").prop("checked",false);
    AlphabethCheckedNum_flag = 0;
    SelectedUserNames = [];
    $("#div_set_password_properties").css("display","grid");
    //здесь должна быть проверка чекбоксов с пользователями
    var tableObj = document.getElementById("table_user_body");
    var checkboxes = tableObj.getElementsByClassName("checkbox");
    for (var index = 0; index < checkboxes.length; index++) {
      if (checkboxes[index].checked) { // положим в массив выбранный
        SelectedUserNames.push(checkboxes[index].id);
//          alert(checkboxes[index].id);
      }
    }
    $("#button_next_third").attr("disabled", true);
    //поведение кнопки "Назад" в шаге установки параметров паролей зависит от первого шага
    if ($("#cbx_set_flag").is(':checked')){
      $("#input_passwords_count").attr("disabled", true);
      $("#button_previous_third").click(callGeneratorSecondStepAction);
      $("#button_next_third").text("СГЕНЕРИРОВАТЬ И УСТАНОВИТЬ");
    }
    else {
      $("#input_passwords_count").attr("disabled", false);
      $("#button_previous_third").click(callGeneratorFirstStepAction);
      $("#button_next_third").text("СГЕНЕРИРОВАТЬ");
    }
}

function callGeneratorFirstStepAction(){
    $("#div_set_users").hide();
    $("#div_set_password_properties").hide();
    $("#div_generate_result").hide();
    //здесь должна быть проверка чекбокса
    $("#div_set_flag").css("display","grid");
    $("#message_generate_password_dialog").text('');
    SelectedUserNames = [];
}

function callGeneratorCgi() {
    $("#print_password_list").attr("disabled","true");
    var urlHost = "http://" + RemoteHostIPAddress + "/cgi-bin/generatorCgi";
    var tableObj = document.getElementById("table_pass_body");
    var cntPass = $("#input_passwords_count").val();
    var cntSym = $("#input_passwords_length").val();
    var alphaFlags = 0;
    if($("#cbx_latin").is(':checked'))
            alphaFlags |= 1;//0b001;
    if($("#cbx_numbers").is(':checked'))
            alphaFlags |= 2;//0b010;
    if($("#cbx_symbols").is(':checked'))
            alphaFlags |= 4;//0b100;
    $.ajax({ 
      url: urlHost,
      header: "Access-Control-Allow-Origin: *",
      method: "POST",
    	data: {"cntPass": cntPass,"cntSymb": cntSym, "numAlph": alphaFlags},
       	success: function(data) {
          let ResultString = data.split('|');
          ResultString.pop();
          if (ResultString.length != 0){
            $("#print_password_list").removeAttr("disabled");
          }
          GPDCreateTable(tableObj, ResultString, false);
        },
        error: function(data) {
          jqcAlert("Отсутствует, либо не настроен генератор паролей");
        }
    });
}

function CheckGeneratorFields(){
   //проверка, если поля пустые
   var OkFlag = false;

   var PasswordsCountVal = parseInt($("#input_passwords_count").val());
   var PasswordsLengthVal = parseInt($("#input_passwords_length").val());

   if ($("#input_passwords_count").val() == '' || $("#input_passwords_length").val() == ''){
       if ($("#input_passwords_count").val() == ''){
           $("#input_passwords_count").val(0);
       }
       if ($("#input_passwords_length").val() == ''){
           $("#input_passwords_length").val(0);
       }
       $("#message_generate_password_dialog").text("Необходимо заполнить все поля");
       $("#message_generate_password_dialog").css("color","#b51212"); 
       OkFlag = false;
   }
   else if (PasswordsCountVal < 1 || PasswordsCountVal > 1200){
      $("#message_generate_password_dialog").text("Количество паролей - не более 1200 и не менее 1.");
      $("#message_generate_password_dialog").css("color","#b51212");
      OkFlag = false;
   }
   else if (PasswordsLengthVal < 8 || PasswordsLengthVal > 100){
      $("#message_generate_password_dialog").text("Количество символов - не более 100 и не менее 8.");
      $("#message_generate_password_dialog").css("color","#b51212");
      OkFlag = false;
   }
   else {
      OkFlag = true;  
   }
    
  if (OkFlag == true){ 
    $("#message_generate_password_dialog").text("");
    $("#message_generate_password_dialog").css("color","#BBB");
    callGeneratorLastStepAction();
  }
}

function callGeneratorLastStepAction(){
    $("#message_generate_password_dialog").text("");
    $("#div_generate_result").css("display","grid");
    $("#div_set_users").hide();
    $("#div_set_flag").hide();
    $("#div_set_password_properties").hide();
    if ($("#cbx_set_flag").is(':checked')){
      $("#ll_pass_table").show();
//      $("#print_password_list").attr("disabled","true");
//      $("#div_generate_result").hide();
        /*очищаем таблицу с паролями, прежде чем заполнить ее снова*/
      var tableObj = document.getElementById("table_pass_body");
      deleteRows(tableObj);
      /*генерируем и устанавливаем пароли*/
      var urlHost = "http://" + RemoteHostIPAddress + "/cgi-bin/generatorCgi";
      var cntPass = SelectedUserNames.length;//пароли по количеству выбранных пользователей
      var cntSym = $("#input_passwords_length").val();
      var alphaFlags = 0;
      if($("#cbx_latin").is(':checked'))
            alphaFlags |= 1;//0b001;
      if($("#cbx_numbers").is(':checked'))
            alphaFlags |= 2;//0b010;
      if($("#cbx_symbols").is(':checked'))
            alphaFlags |= 4;//0b100;
//        alert(alphaFlags);
      $.ajax({ 
        url: urlHost,
        header: "Access-Control-Allow-Origin: *",
        method: "POST",
        data: {"cntPass": cntPass,"cntSymb": cntSym, "numAlph": alphaFlags},
       	success: function(data) {
          let PasswordsFromCgi = data.split('|');
          PasswordsFromCgi.pop();
          let PasswordMap = [];
          if (SelectedUserNames.length == PasswordsFromCgi.length){
            for (var i = 0; i < PasswordsFromCgi.length; i++){
              let UserPasswordObject = new Object();
              UserPasswordObject = {
                name: SelectedUserNames[i],
                password: PasswordsFromCgi[i]
               };
              PasswordMap.push(UserPasswordObject);
            }
            SetPasswords(PasswordMap); 
          }
            else {
              jqcAlert("Число пользователей не соответствует числу паролей от cgi");  
            }
        },
        error: function(data) {
          jqcAlert("Отсутствует, либо не настроен генератор паролей");
        }
      });
    }
    else {     
//      $("#div_generate_result").css("display","grid");
      callGeneratorCgi();
    }
}

function callCloseGeneratePasswordDialog(){
    $("#div_set_users").hide();
    $("#div_set_password_properties").hide();
    $("#div_generate_result").hide();
    $("#message_generate_password_dialog").text("");
    $("#cbx_set_flag").prop("checked", false);
    $("#cbx_latin").prop("checked", false);
    $("#cbx_numbers").prop("checked", false);
    $("#cbx_symbols").prop("checked", false);
    //здесь должна быть проверка чекбокса
    $("#div_set_flag").css("display","grid");
//    GetUsersALD();//обновим список пользователей, чтобы посмотреть дату последней смены пароля
    $(this).closest('.ui-dialog-content').dialog('close');
}

function PrintGeneratedPasswordList(){
    //Печать списка паролей по кнопке "Печать" в диалоге. Привязка кнопки и события при загрузке страницы
    $("#table_pass_body").printThis();
}

var RowCheckedCounter = 0;
function GPDCreateTable(table, $tdTextArray, showCheckBoxFlag){
    deleteRows(table);
//    alert(tdTextArray);
    for (var i = 0; i < $tdTextArray.length; i++){
      var trObj = document.createElement("TR");
      var tdObj = document.createElement("TD");
      tdObj.align = "center";
      tdObj.valign = "bottom";
      tdObj.id = "td_id";
      if (showCheckBoxFlag){
        var check = document.createElement("input");
        check.className = "checkbox";
        check.type = "checkbox";
        check.align = "left";
        check.id = $tdTextArray[i];
        check.addEventListener("click", function() {
        ($(this).is(':checked'))? RowCheckedCounter++ : RowCheckedCounter--;
        (RowCheckedCounter > 0) ?  $("#button_next_second").removeAttr("disabled"): $("#button_next_second").attr("disabled","true");//только для таблицы с пользователями в генераторе паролей
        });
        tdObj.appendChild(check);
       }
      tdObj.appendChild(document.createTextNode($tdTextArray[i]));
      trObj.appendChild(tdObj);
      table.appendChild(trObj);
    }
}
