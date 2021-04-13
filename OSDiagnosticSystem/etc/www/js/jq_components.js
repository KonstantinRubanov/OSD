/**
 * Замена для стандартной alert - не зависит от желания пользователя (показывать или нет)
 * @param {*} outputMsg Выводимое сообщение
 * @param {*} titleMsg Заголовок
 * @param {*} onCloseCallback Функция после закрытия alert'a
 */
function jqcAlert(outputMsg, titleMsg, onCloseCallback) {
  if (!titleMsg) {
    titleMsg = 'Предупреждение';
  }
  if (!outputMsg) {
    outputMsg = 'Предупреждение без текста...';
  } 
  $("<div></div>").html(outputMsg).dialog({
    dialogClass: "jqc_alert",
    title: titleMsg,
    resizable: false,
    modal: true,
    buttons: {
      "OK": function() {
        $( this ).dialog( "destroy" );
      }
    },
    close: onCloseCallback
  });
}