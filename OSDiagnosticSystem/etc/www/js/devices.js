var g_checked_rec = false;
var update_object = 0;
var timer = 0;

var Hearttimer = 0;

function formatDate(date) {
  try{
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

$(document).ready(function(){
   hostlistAld();
});

function LoadTemplateContentAuditFiles(mHost, mAction, mFrameType){  
  $(".auditblock_labels .rules_hidden_dir").html("");
  var button_audit_add = $(".auditblock_labels .rules_audit_button").html();
  $(".auditblock_labels .rules_audit_button").html("");
  var host =  $('.resource_host').text();
  $('.main_audit_div_folder').after(button_audit_add);
  DirAction(host,"/", mFrameType);
}


function TemplateUsbPropUpdateData(ListOfNames){
  
  /*$(".rule_ald_device_tmp_cols_1").html(function(index, oldText){     
     oldText = ListOfNames.data[0].[index];    
    return oldText;
  });*/    
  $(".rule_ald_device_tmp_cols_2").html(function(index, oldText){     
    oldText = ListOfNames.data[0].serial[index];    
    return oldText;
  });  
  
  $(".rule_ald_device_tmp_cols_3").html(function(index, oldText){     
    oldText = "ENV{ID_SERIAL_SHORT}==\"" + ListOfNames.data[0].serial[index] + "\"";    
    return oldText;
  });

  $(".rule_ald_device_tmp_cols_4").html(function(index, oldText){    
    oldText = "<span id=\"glowtext\"><img name=\"ruledeldevedomain\" class=\"system_image\" id=\"" + ListOfNames.data[0].serial[index] + "\" src=\"images/delete_icon.png\"></span>";    
    return oldText
  });
  
  $('img[name=ruledeldevedomain]').click(function() {     
      var id = $(this).attr('id'); 
      var prop_mode = $('button[name=ald_save_user_usb_group]').attr('id');
      var cnt_group = 0;
      $(".rule_ald_device_tmp_cols_2").parent().html(function(index, oldText){     
          cnt_group = cnt_group + 1;  
          return oldText;
      });
      if(cnt_group <= 1){
          alert('Для устройства должно быть описано по крайней мере 1 правило.\r\nНе допускается описание устройства без правил.\r\nЕсли вы хотите удалить данное правило (единственное) требуется полностью удалить устройство и создать его с новым правилом! Также можно удалить данное правило после добавления другого правила.');
          return;
      }
      
      
       if(prop_mode == 2 ){ 
          dev_name_id = $('#_ald_usb_name').val();
          DomaiinTemplateRuleDelete(dev_name_id, id);          
    }
  });
  
  $('#rule_host_list_all_div').dialog('open');

}

function LoadTemplateDataPolicy(ListOfNames){
  var cnt_dev = 0;
   $(".ald_device_tmp_cols_1").html(function(index, oldText){     
    oldText = index + 1;    
    return oldText;
  }); 
  $(".ald_device_tmp_cols_2").html(function(index, oldText){ 
    oldText = ListOfNames.data[index].name;    
    return oldText;
  }); 
  $(".ald_device_tmp_cols_4").html(function(index, oldText){ 
    oldText = ListOfNames.data[index].owner;    
    return oldText;
  }); 
  $(".ald_device_tmp_cols_5").html(function(index, oldText){ 
    oldText = ListOfNames.data[index].group;    
    return oldText;
  }); 
 $(".ald_device_tmp_cols_6").html(function(index, oldText){ 
    oldText = ListOfNames.data[index].info;    
    return oldText;
  }); 
  $(".ald_device_tmp_cols_7").html(function(index, oldText){ 
    oldText = ListOfNames.data[index].rule;    
    return oldText;
  }); 
 $(".ald_device_tmp_cols_8").html(function(index, oldText){ 
     if(ListOfNames.data[index].state == "1")
       oldText = "<span id=\"glowtext\"><img  class=\"system_image\" id=\"" + ListOfNames.data[index].name + "\" src=\"images/task-complete.png\"></span>";    
     return oldText;
  }); 
 $(".ald_device_tmp_cols_9").html(function(index, oldText){ 
    oldText = "<span id=\"glowtext\"><img  name=\"updateudev\" class=\"system_image\" id=\"" + ListOfNames.data[index].name + "\" src=\"images/settings-icon.png\"></span>";    
    return oldText;
  }); 
  $(".ald_device_tmp_cols_10").html(function(index, oldText){ 
    oldText = "<span id=\"glowtext\"><img name=\"deldevedomain\" class=\"system_image\" id=\"" + ListOfNames.data[index].name + "\" src=\"images/delete_icon.png\"></span>";    
    return oldText;
  }); 
  $('img[name=deldevedomain]').click(function() {     
      var id = $(this).attr('id'); 
      var resIs = "Вы действительно хотите удалить правила для выбранного устройства?\r\n" + id;
      var isYes = confirm(resIs);
      if(isYes){
        DelhostlistAldPolicy(id);
      }
  });
  $('img[name=updateudev]').click(function() {                 
      var id = $(this).attr('id');
      $(".rule_ald_device_row_next").html(function(index, oldText){     
          $(this).remove();
          return oldText;
      });                
      $('.rule_tb_ald_list_tmpl tr:last').after("<tr class=\"rule_ald_device_row_next\"><td class=\"rule_ald_device_tmp_cols_2\" id=\"\"></td>\<td class=\"rule_ald_device_tmp_cols_3\" id=\"\"></td><td class=\"rule_ald_device_tmp_cols_4\" id=\"\"></td></tr>").ready(function () {      
          ViewhostlistAldPolicy(id);
      });      
      
  });

}

function UpdateSaveDataForTemplate(ListOfNames){
  $(".tmp_cols_1").html(function(index, oldText){ 
    oldText = "<span id=\"glowtext\"><img  class=\"system_image\" id=\"folder_img_gr\" src=\"images/Folder-icon.png\">" + ListOfNames.data[index] + "</span>";    
    return oldText;
  }); 
  $("img[name=mod_save_clear]").html(function(index, oldText){ 
    $(this).prop('id', ListOfNames.data[index]);   
    return oldText;
  }); 
  $("img[name=mod_save_upload]").html(function(index, oldText){ 
    $(this).prop('id', ListOfNames.data[index]);   
    return oldText;
  }); 
  $("img[name=mod_save_delete]").html(function(index, oldText){ 
    $(this).prop('id', ListOfNames.data[index]);   
    return oldText;
  }); 
  $('img[name=mod_save_delete]').click(function() {          
      var id = $(this).attr('id');       
      var host =  $('.resource_host').text();
      var resIs = "Вы действительно хотите удалить ресурс " + id + "  на хосте " + host + " из списка защищаемых?";
      var isYes = confirm(resIs);
      if(isYes){
         SaveResourceDiskModeAction(host, id, 'd', '');
      }
  });
  
  $('img[name=mod_save_clear]').click(function() {          
      var id = $(this).attr('id');       
      var host =  $('.resource_host').text();
      var resIs = "Вы действительно хотите очистить данные в каталоге " + id + "  на хосте " + host + "? Данная операция является необратимой!";
      var isYes = confirm(resIs);
      if(isYes){
         SaveResourceDiskModeAction(host, id, 'c', '');
      }
  });
  
   $('img[name=setttings_save_allempty]').click(function() {          
      var id = $(this).attr('id');       
      var host =  $('.resource_host').text();
      var resIs = "Вы действительно хотите очистить данные во всех защищаемых каталогах? Данная операция является необратимой!";
      var isYes = confirm(resIs);
      if(isYes){
         SaveResourceDiskModeAction(host, id, 'c', 'R');
      }
  });
   
  $('img[name=setttings_save_allexport]').click(function() {     
      var host =  $('.resource_host').text();
      var resIs = "Вы действительно хотите выполнить экспорт данных всех защищаемых ресурсов?";
      var isYes = confirm(resIs);
      if(isYes){
        ProtectedActionListOperation(host, 'all', 1, 'R');
      }
  });
   
    $('#table_backup_dialog','.ui-dialog').remove();    
	$('img[name=mod_save_upload]').click(function(e) {
      var id = $(this).attr('id');       
      var host =  $('.resource_host').text();      
       $('#table_backup_dialog').dialog({ 
                           width:  800,
                           height: 400            
    });  
       ProtectedActionList(host, id, 0);
    }); 
    
    $('#_server_save','.ui-dialog').remove();    
	$('img[name=setttings_save_delete]').click(function(e) {
      var id = $(this).attr('id');       
      var host =  $('.resource_host').text();      
      $('#_server_save').dialog({ 
                           width:  400,
                           height: 350            
    });  
      
      SaveSettingsAction();
   }); 
    
   $('#_server_save').on('dialogclose', function(event) {
     SaveSettingsAction();      
   }); 
    
   $('button[name=save_pro_settings]').click(function() {          
     var mServer = $('#_server').val();
     var mUser = $('#_login').val();
     var mPass = $('#_password').val();
     UpdateSaveSettingsAction(mServer, mUser, mPass);
  });
    
    
}

function LoadTemplateDataDevCD(mHost, ListOfNames){ 
   ResourceLocalObjectsSelect(mHost,'Users','#audit_rule_who_id_',0,0);
   $('img[name=rem_from_group]').html(function(index, oldText){  
      $(this).prop('id', ListOfNames.data[index]);
   }); 
   $('.device_tmp_cols_1').html(function(index, oldText){  
      oldText = ListOfNames.data[index];
      return oldText;
    });  
   $('img[name=rem_from_group]').click(function() {          
      var id = $(this).attr('id');       
      var host =  $('.resource_host').text();
      ModifyCDROMActionList(mHost,'cdrom', id, 'd');
   });
   $('#save_user_cd_group').click(function() {         
       var id_user = $('#users').children('select').find("option:selected").attr('value'); 
       var host =  $('.resource_host').text();
       ModifyCDROMActionList(mHost,'cdrom', id_user, 'a');
   });
   
    $('#devbus_rule_who_id_').change(function() {   
      var host =  $('.resource_host').text();
      var id_dev = $('#devbus_rule_who_id_').find("option:selected").attr('value');
      //alert(id_dev);
      if(id_dev == 'busCD')
        CDROMActionList(host, 'cdrom');
      if(id_dev == 'busUSB')
        USBActionList(host, 'usb_storage','list');
      if(id_dev == 'rulesUSB')
        USBRulesActionListNeedTouch(host, '','', 0);
   });
   
}


function LoadTemplateDataDevUSB(mHost, ListOfNames){ 
    var is_storage = false;
    
     $('input[name=mod_switch]').html(function(index, oldText){  
       if(ListOfNames.data[index] == 'usb_storage')
         $(this).prop('checked',true);
       return oldText;
    })
        
    $('input[name=mod_switch]').change(function(){ 
        var id = $(this).attr('id'); 
        var host =  $('.resource_host').text();        
        
        if($(this).is(':checked'))
          USBActionList(host, id,'enable');
        else
          USBActionList(host, id,'disable');                    
    });   

    $('#devbus_rule_who_id_').change(function() {   
      var host =  $('.resource_host').text();
      var id_dev = $('#devbus_rule_who_id_').find("option:selected").attr('value');
      if(id_dev == 'busCD')
        CDROMActionList(host, 'cdrom');
      if(id_dev == 'busUSB')
        USBActionList(host, 'usb_storage','list');  
      if(id_dev == 'rulesUSB')
        USBRulesActionListNeedTouch(host, '','', 0);
   });
   
}


function LoadTemplateDataDevUSBRules(mHost, ListOfNames){ 
    var is_storage = false;
    var rules = 0;
    $('.device_tmp_cols_4').html(function(index, oldText){  
      var rule =  ListOfNames.data[index].rule;
      if(!rule.length) oldText = "";
	  return oldText;
    });
    $('img[name=mod_drive_disable]').html(function(index, oldText){  
       $(this).prop('id', ListOfNames.data[index].serial);   
       return oldText;
    }); 
   $('.device_tmp_cols_1').html(function(index, oldText){  
      rules = rules + 1;
      if(ListOfNames.data[index].drive == "UNKNOWN")
        oldText = "<span id=\"glowtext\"><img class=\"system_image\" src=\"images/usb_off.png\"> не подключено</span>";        
      else
        oldText = "<span id=\"glowtext\"><img class=\"system_image\" src=\"images/usb.png\"> подключено: " + ListOfNames.data[index].drive + "</span>";
      if(ListOfNames.data[index].drive != "UNKNOWN")
        $('#usb_hard_rule_who_id_').append("<option value=\"" + ListOfNames.data[index].serial +"\">" + ListOfNames.data[index].drive + "</option>");
      return oldText;
    });	
    if(!rules)
      $('.header_state_storage').html("<font color=\"red\"><h4>Не обнаружено подключенных usb-устройств.</h4></font>");           
    else
      $('.header_state_storage').html("<h4>Перечень usb-устройств и правил udev:</h4>");       
	$('.device_tmp_cols_2').html(function(index, oldText){  
      oldText = ListOfNames.data[index].serial;
	  return oldText;
    });
  	$('.device_tmp_cols_3').html(function(index, oldText){  
      var rule = ListOfNames.data[index].rule;
      //if(rule == "1") oldText = "Разрешено";
      //if(rule == "0") oldText = "Запрещено";
      if(!rule.length)
        oldText = "Правило не определено";
      else
        oldText = "Правило назначено\r\n" + rule;
      //oldText = ListOfNames.data[index].rule;
	  return oldText;
    });    
    $('.device_tmp_cols_vendor').html(function(index, oldText){  
      oldText = "USB DISK";
	  return oldText;
    });
    $('img[name=mod_drive_disable]').click(function(){ 
        var id = $(this).attr('id'); 
        var host =  $('.resource_host').text();    
        USBRulesActionList(mHost, '', id, 1, '', '', 0, 0);                    
    });   
  
    $('button[name=save_user_usb_group]').click(function() {                  
        $('#users_usb_device').dialog('close');
        var host =  $('.resource_host').text();
        var id_dev = $('#usb_hard_rule_who_id_').find("option:selected").attr('value');
        var id_user  = $('#audit_rule_who_id_').find("option:selected").attr('value'); 
        var id_group = $('#audit_rule_group_id_').find("option:selected").attr('value');
        var bValueU = ($('#s_owner_user_usb').is(':checked'))?"1":"0";  
        if(bValueU == "0"){
          id_user="";
          id_group="";
        }
          
        var AclDiscreteVal = 0;
        var cnt_Dscr = 9;
        var bit_num;
        while(cnt_Dscr--){
          bit_num = document.getElementById(("discrete_bit_" + cnt_Dscr.toString()));
          if ($(bit_num).is(":checked")){
            AclDiscreteVal|=(1<<cnt_Dscr);
          }
        }
        var octVal =   (AclDiscreteVal).toString(8); 
        if(bValueU == "0") octVal = 0;
        USBRulesActionList(mHost, '', id_dev, 2, id_user, id_group, octVal, 0);         
    });
    
    $('#devbus_rule_who_id_').change(function() {   
      var host =  $('.resource_host').text();
      var id_dev = $('#devbus_rule_who_id_').find("option:selected").attr('value');
      //alert(id_dev);
      if(id_dev == 'busCD')
        CDROMActionList(host, 'cdrom');
      if(id_dev == 'busUSB')
        USBActionList(host, 'usb_storage','list');  
      if(id_dev == 'rulesUSB')
        USBRulesActionListNeedTouch(host, '','', 0);
   });
    
      
    $('button[name=ins_user_usb_update]').click(function() {                  
        var host =  $('.resource_host').text();
        USBRulesActionListNeedTouch(host, '','', 0);
    });
    
    $('button[name=del_usb_file_rule]').click(function() {
        var host =  $('.resource_host').text();
        var resIs = "Вы хотите удалить на хосте локальный файл правил UDEV?";
              var isYes = confirm(resIs);
              if(isYes){
                  UdevLocalClear(host);    
              }
    });
   
}

function LoadTemplateContentAuditFilesMain(mHost, mAction, ListOfNames, mConfig){  
  $(".auditblock_labels").html("");
  var host =  $('.resource_host').text();
  var cnt = 0;  
  var HeaderList = "";
  /*headers fields*/
  if(mConfig == "") 
    $('#header_block').append("<span id=\"glowtext\" onclick=\"CheckLoader(this);hostlistAld();\"><img class=\"system_image\" id=\"back_go\"  onclick=\"Check(this);\" src=\"images/back.png\">Вернуться к списку устройств</span>").ready(function () {      
  });
  else
    $('#header_block').append("<span id=\"glowtext\" onclick=\"CheckLoader(this);AuditAction('" + mHost + "','/','');\"><img  class=\"system_image\" id=\"back_go\"  src=\"images/back.png\" onclick=\"Check(this);\">Вернуться к списку аудита " + mHost + "</span><br>").ready(function () {      
  });
  var rec_check = (g_checked_rec == true)?"checked":"";
  if(mConfig != "") 
    $('#header_block').append("<span><p id=\"controls_rec\"><input type=\"checkbox\" id=\"s_recursive\" value=\"recursive\" " + rec_check + "> Применить рекурсию</p></span>").ready(function () {      
  });  
  
  if(mConfig != "")
    $('#pathtable_line tr:last').html("<tr></tr>").ready(function () {      
  }); 
    
  /*audit fields*/  
  if(mConfig == ""){ 
    $('.tmp_cols_1').html(function(index, oldText){  
      oldText = ListOfNames.data[index].res_hame;
      return oldText;
    });
    $('.tmp_cols_2').html(function(index, oldText){  
      oldText = "<span id=\"glowtext\" onclick=\"CheckLoader(this);AuditAction(\'" + mHost + "\',\'" + ListOfNames.data[index].res_hame + "\',\'" + ListOfNames.data[index].res_attr + "\');\"><img class=\"system_image\" src=\"images/settings-icon.png\" onclick=\"Check(this);\"></span>";
      return oldText;
    });
    $('.tmp_cols_3').html(function(index, oldText){   
      oldText = "<span id=\"glowtext\" name=\"jurnalview\" onclick=\"AuditJournalAction(\'" + mHost + "\',\'" + ListOfNames.data[index].res_hame + "\');\"><img class=\"system_image\" src=\"images/state_write.png\"></span>";
      return oldText;
    });
    $('.tmp_cols_4').html(function(index, oldText){    
      oldText = "<span id=\"glowtext\" onclick=\"AuditActionAdd(\'" + mHost + "\',\'" + ListOfNames.data[index].res_hame + "\',\'-\',true,\'\');\"><img class=\"rule_remove_audit\" id=\"#remove_audit\" name=\"rem_audit\" src=\"images/delete_icon.png\"/></span>";
      return oldText;
    });
  }  
}

function LoadTemplateContentAuditList(mHost, mAction, mListOfAudit,block) {
  var header = $(".auditblock_labels .rules_audit_header").find( $("tbody > tr.rule_header_th")).html();
  var rule   = $(".auditblock_labels .rules_audit_header").find( $("tbody > tr.rule_row_next")).html();  
  var block  = $(".auditblock_labels .add_audit_div");  
  var block_bt  = $(".auditblock_labels .add_audit_bt_mod_div"); 
  $(".auditblock_labels").html("");  
  var cnt = 0;  
  $('#pathtable_line tr:last').after("<tr>" + header +"</tr>");
  if(mListOfAudit != "")
  for(k in  mListOfAudit){  
    $('#pathtable_line tr:last').after("<tr>" + rule + "</tr>");
    cnt++;
  }
  $('#pathtable_line').after(block_bt);
  var  img_c = 0;
  $('.rule_remove_audit').each(function () {
    var del_rule = "";
    if(mListOfAudit[img_c].subject_rule.toString() == "o")
      del_rule = "o:0:0";
    else
      del_rule = "\'" + mListOfAudit[img_c].subject_rule.toString()+"\':\'"+mListOfAudit[img_c].name_rule.toString()+"\':0:0"; 
    $(this).prop('id', del_rule); 
    img_c = img_c + 1;
   });
  /**/
  $(".rule_type_obj").text(function(index, oldText) {
    var sb = "-";
    if(mListOfAudit[index].subject_rule == "u"){
      sb = "Пользователь";
    }
    else if(mListOfAudit[index].subject_rule == "g"){
      sb = "Группа";
    }
    else if(mListOfAudit[index].subject_rule == "o"){
      sb = "Все";
    }            
    oldText = sb.toString();
    return oldText;
  });
  /**/
  $(".rule_name_obj").text(function(index, oldText) {
    var name_rule = "-"
    if(mListOfAudit[index].subject_rule == "u"){
      name_rule = mListOfAudit[index].name_rule;
    }
    else if(mListOfAudit[index].subject_rule == "g"){
      name_rule = mListOfAudit[index].name_rule;
    }
    else if(mListOfAudit[index].subject_rule == "o"){
      name_rule = "-";
    }            
    oldText = name_rule.toString();
    return oldText;
  });
    /**/
  $(".rule_text_obj").text(function(index, oldText) {
    oldText = mListOfAudit[index].success_labels.toString() + ":" + mListOfAudit[index].fail_labels.toString();      
    return oldText;
  });       
  $('.auditblock_labels').append(block).ready(function () {
      
  });  
  
  UpdateRulesEditForm ();
  //if(cnt > 0){; 


 // }
}

function hostlistAld() {
 //alert('hostlist');  
  CheckHeartTimer();
 //$("#ResourceCurrent").html("<img class=\"system_image\" id=\"file_go\" src=\"images/loader.gif\"><h3>Обновление списка устройств...</h3>");
   $('#ResourceCurrent').html(function(index, oldText){ 
    oldText    = "<span id=\"glowtext\"><img class=\"system_image\" src=\"images/loader.gif\"><h3>Обновление списка устройств...</h3></span>";
    return oldText;
  });    
  var xmlhttp = getXmlHttp(); // Создаём объект XMLHTTP
  xmlhttp.timeout = 10000;
  xmlhttp.ontimeout = function() {
    xmlhttp.abort();
    //SshTouchStripts();
    //alert("Данные не получены от сервера (Timeout request).");    
  }  
  xmlhttp.open('POST', '/ald', true); // Открываем асинхронное соединение
  xmlhttp.setRequestHeader('Cache-Control','max-age=3600');
  xmlhttp.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded');    
  
  xmlhttp.send("hostsAld=hostlist");  
  xmlhttp.onreadystatechange = function() { // Ждём ответа от сервера
    if (xmlhttp.readyState == 4) { // Ответ пришёл        
      if(xmlhttp.status == 200) { // Сервер вернул код 200 (что хорошо)
          $("#ResourceCurrent").html(""); 
        var ListOfNames = JSON.parse(xmlhttp.responseText);
          if(ListOfNames.state != 0){
            $('#tableHostPrint').html("");   
            $('#tableHostPrint').html("Ошибка получения данных от сервера:<br><h3>" + ListOfNames.data.update + "</h3>"); 
            return;
          }            
          $('#tableHostPrint').html("");                            
          $('#tableHostPrint').load('html/host_tmp_devices.html', '.host_list_all_div', function(){                         
            var out = "";
            var iDivObj = 0;
            var iTimeObj = 0;		 
		    var NamesData   = ListOfNames.data;
            var device_tmpl  = $(".tb_hosts_list_tml").find( $("tbody > tr.device_row_next")).html();
            $(".tb_hosts_list_tml").find( $("tbody > tr.device_row_next")).remove();
            for(i in ListOfNames.data) { 
              $('.tb_hosts_list_tml tr:last').after("<tr>" + device_tmpl + "</tr>");
              iDivObj += 1;
            }
            LoadTemplateContentHosts(ListOfNames);  
            $(".tb_hosts_list_tml").find( $("tbody > tr.device_row_")).remove();
            //var table = $('.tb_hosts_list_tml').DataTable();
          });  
      }
      else if (xmlhttp.status == 401) { // Нет авторизации
        location.href = '/';
      }
      else {
        //обработка ошибки
        if(this.statusText.length)
          alert('error: ' + this.status ? this.statusText : 'query failed');
        return;
      }
    }
  };
}


function SshTouchStripts() {
  var xmlhttp = getXmlHttp(); // Создаём объект XMLHTTP
  xmlhttp.timeout = 10000;
  xmlhttp.ontimeout = function() {
    xmlhttp.abort();
    //alert("Данные не получены от сервера (Timeout request).");
  }
  xmlhttp.open('POST', '/action', true); // Открываем асинхронное соединение
  xmlhttp.setRequestHeader('Cache-Control','max-age=3600');
  xmlhttp.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded');  
  var Resp = "resourcebyhost=sshd_check";
  xmlhttp.send(Resp);     
  xmlhttp.onreadystatechange = function() { // Ждём ответа от сервера
    if (xmlhttp.readyState == 4) { // Ответ пришёл        
      if(xmlhttp.status == 200) { // Сервер вернул код 200 (что хорошо)          
            //alert(xmlhttp.responseText);     
            hostlistAld();
         }
      }
      else if (xmlhttp.status == 401) { // Нет авторизации
        location.href = '/';
      }
      else {
       //обработка ошибки
       // if(this.statusText.length)
       //alert('error: ' + this.status ? this.statusText : 'query failed');
        return;
      }
    }
  }


function CheckHeartTimer(){
  //alert("timer...");
  var objectHeart = this;
  var first_time  = 3000;
  var next_time   = 10000;
  Hearttimer      = 0;
  this.start = function() {
    //alert("start");
    Hearttimer = setInterval(objectHeart.tick, first_time);
  }
  this.stop = function() {
    clearInterval(Hearttimer);
    //alert("stop");
    Hearttimer = 0;
  }
  this.tick = function() {
    //alert("tick");
    var cnt_ping = 0;
    var cnt_elem = 0;
    
    var test     = $('#tab_devices').is(':checked')?"1":"0"; 
    var mn_page  = $('button[name=dev_main_upd]').prop('onclick');
    
    if(test == 1 && mn_page!= undefined){
      DomainFindDevAlive();
      clearInterval(Hearttimer);
      Hearttimer = setInterval(objectHeart.tick, next_time);
    }
    else
     objectHeart.stop();

   /*  $('.device_tmp_cols_2').html(function(index, oldText){                                                 
      //DomainFindDevAlive(oldText, index);
      return oldText;
   // });
      
      
    $('.device_tmp_cols_6').html(function(index, oldText){                                                 
      if(oldText == "" || oldText == undefined)  {}
      else{
        //alert(oldText);
        cnt_ping = cnt_ping + 1;        
      }
      cnt_elem = cnt_elem + 1;
      return oldText;
    });
    //alert(cnt_ping);
    if((cnt_elem > 0 && cnt_ping == 0) || (mn_page!= undefined && go_state != undefined)){
      //alert("need reload....");
      SshTouchStripts();
    }
    */
    
  }  
  this.start();
}  
  

 
function take_obj(mObj){
  $(document.getElementById(mObj)).slideToggle("slow");
}


function DirAction(mHost, mAction, mFrameType) {
  //alert('hostResources!');  
  $(".rules_hidden_dir").html("<img class=\"system_image\" id=\"file_go\" src=\"images/loader.gif\"><h3>Обновление списка файлов...</h3>")
  var xmlhttp = getXmlHttp(); // Создаём объект XMLHTTP
  xmlhttp.timeout = 60000;
  xmlhttp.ontimeout = function() {
    xmlhttp.abort();
    alert("Данные не получены от сервера (Timeout request).");
  }
  xmlhttp.open('POST', '/action', true); // Открываем асинхронное соединение
  xmlhttp.setRequestHeader('Cache-Control','max-age=3600');
  xmlhttp.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded');  
  var Resp = "resourcebyhost=resourcelist";
  if(mHost.length)
    Resp += "&resourcehost=" + mHost;
  if(mAction.length)
    Resp += "&resourcepath=" + mAction;
  if(mFrameType.length)
    Resp += "&resourcetype=" + mFrameType;
  
  xmlhttp.send(Resp);   
  xmlhttp.onreadystatechange = function() { // Ждём ответа от сервера
    if (xmlhttp.readyState == 4) { // Ответ пришёл        
      if(xmlhttp.status == 200) { // Сервер вернул код 200 (что хорошо)          
          //alert(xmlhttp.responseText);          
          var url = mAction.substring(0, mAction.lastIndexOf('/'));
          url = url.substring(0, url.lastIndexOf('/'))
          
;         var out = "";
          var iDivObj = 0;
		  var ListOfNames = JSON.parse(xmlhttp.responseText);
          //alert(xmlhttp.responseText);
		  var NamesData   = ListOfNames.data;  
          
          $('.rules_hidden_dir').html("");          ;
          $('.rules_hidden_dir').load('html/dir_file_tmp_devices_inner.html', '.tmpl_file_list_dir_div', function(){              	 
		    var NamesData   = ListOfNames.data;            
            var file_row_tmpl  = $(".tb_file_list_tmpl").find( $("tbody > tr.tmp_file_row_next")).html();           
            for(i in ListOfNames.data) { 
              if(ListOfNames.data[i].res_hame != ".."){
                $('.tb_file_list_tmpl tr:last').after("<tr>" + file_row_tmpl + "</tr>");
                iDivObj += 1;
              }
            }     
            LoadTemplateContentDir(mHost, url, ListOfNames, mFrameType);
            if(mFrameType != "dir")            
              UpdateRulesEditForm ();
            else
              UpdateRulesSaveEditForm();
          });           

      }
      else if (xmlhttp.status == 401) { // Нет авторизации
        location.href = '/';
      }
      else {
        //обработка ошибки
        if(this.statusText.length)
          alert('error: ' + this.status ? this.statusText : 'query failed');
        return;
      }
    }
  };
}


function hostAction(mHost, mAction) {
  //alert('hostResources!');
  $("#ResourceCurrent").html("<img class=\"system_image\" id=\"file_go\" src=\"images/loader.gif\"><h3>Обновление списка файлов...</h3>");
  var xmlhttp = getXmlHttp(); // Создаём объект XMLHTTP
  xmlhttp.timeout = 60000;
  xmlhttp.ontimeout = function() {
    xmlhttp.abort();
    alert("Данные не получены от сервера (Timeout request).");
  }
  xmlhttp.open('POST', '/action', true); // Открываем асинхронное соединение
  xmlhttp.setRequestHeader('Cache-Control','max-age=3600');
  xmlhttp.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded');  
  if(mAction.length)
    xmlhttp.send("resourcebyhost=resourcelist&resourcehost=" + mHost + "&resourcepath=" + mAction); 
  else
    xmlhttp.send("resourcebyhost=resourcelist&resourcehost=" + mHost);   
  xmlhttp.onreadystatechange = function() { // Ждём ответа от сервера
    if (xmlhttp.readyState == 4) { // Ответ пришёл        
      if(xmlhttp.status == 200) { // Сервер вернул код 200 (что хорошо)          
          //alert(xmlhttp.responseText);          
          $("#ResourceCurrent").html(""); 
          var url = mAction.substring(0, mAction.lastIndexOf('/'));
          url = url.substring(0, url.lastIndexOf('/'))
          
;         var out = "";
          var iDivObj = 0;
		  var ListOfNames = JSON.parse(xmlhttp.responseText);
          //alert(xmlhttp.responseText);
		  var NamesData   = ListOfNames.data;        
          var HeaderList = "<br><br><span id=\"glowtext\" onclick=\"CheckLoader(this);hostlistAld();\"><img class=\"system_image\" id=\"back_go\"  src=\"images/back.png\">Вернуться к списку устройств</span>";
          
          
          $('#tableHostPrint').html("");          ;
          $('#tableHostPrint').load('html/dir_file_tmp_devices.html', '.tmpl_file_list_dir_div', function(){              	 
		    var NamesData   = ListOfNames.data;            
            var file_row_tmpl  = $(".tb_file_list_tmpl").find( $("tbody > tr.tmp_file_row_next")).html();           
            for(i in ListOfNames.data) { 
              if(ListOfNames.data[i].res_hame != ".."){
                $('.tb_file_list_tmpl tr:last').after("<tr>" + file_row_tmpl + "</tr>");
                iDivObj += 1;
              }
            }     
            LoadTemplateFilesDevuce(mHost, url, ListOfNames);
          }); 
                    
           if(mAction == "/") mAction = "";
           $('#ResourceCurrent').html("<span>Права доступа к ресурсам: " + mHost + "  " + mAction + HeaderList + "</span>");  

      }
      else if (xmlhttp.status == 401) { // Нет авторизации
        location.href = '/';
      }
      else {
        //обработка ошибки
        if(this.statusText.length)
          alert('error: ' + this.status ? this.statusText : 'query failed');
        return;
      }
    }
  };
}




function SaveResourceAction(mHost) {
  //alert('hostResources!');    
  $("#tableHostPrint").html("<img class=\"system_image\" id=\"file_go\" src=\"images/loader.gif\"><h3>Обновление списка защищаемых ресурсов...</h3>");
  var xmlhttp = getXmlHttp(); // Создаём объект XMLHTTP
  xmlhttp.timeout = 60000;
  xmlhttp.ontimeout = function() {
    xmlhttp.abort();
    alert("Данные не получены от сервера (Timeout request).");
  }
  xmlhttp.open('POST', '/action', true); // Открываем асинхронное соединение
  xmlhttp.setRequestHeader('Cache-Control','max-age=3600');
  xmlhttp.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded');  
  xmlhttp.send("resourcebyhost=savelist&resourcehost=" + mHost);   
  xmlhttp.onreadystatechange = function() { // Ждём ответа от сервера
    if (xmlhttp.readyState == 4) { // Ответ пришёл        
      if(xmlhttp.status == 200) { // Сервер вернул код 200 (что хорошо)                    ;
          //alert(xmlhttp.responseText);
          $("#ResourceCurrent").html("");           
;         var out = "";
          var iDivObj = 0;
		  var ListOfNames = JSON.parse(xmlhttp.responseText);
          //alert(xmlhttp.responseText);
		  var NamesData   = ListOfNames.data;
          $("#tableHostPrint").html("");
          for(i in ListOfNames.data) {                                      
             iDivObj += 1;
          }                         
          if(iDivObj == 0) pathObj = "..";;                    
          $('#tableHostPrint').load('html/save_tmp_devices.html', '.main_audit_div_folder', function(){                
             ///XXX 
            var row_tmpl  = $(".main_audit_div_folder").find( $("tbody > tr.tmp_row_next")).html();
            $(".main_audit_div_folder").find( $("tbody > tr.tmp_row_next")).remove();
             for(i in ListOfNames.data) { 
              $('#pathtable_line tr:last').after("<tr>" + row_tmpl + "</tr>");
            }
            $('.file_dialog').load('html/audit_tmp_files.html', '.rules_audit_div_folder', function(){  
                LoadTemplateContentAuditFiles(mHost, '/', 'dir');                                
            });  
            
            UpdateSaveDataForTemplate(ListOfNames);
         });
          document.getElementById("ResourceCurrent").innerHTML = "<span>Список защищаемых ресурсов: <p class=\"resource_host\">" + mHost + "</p></span>";       
         // }            

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





function SaveResourceDiskModeAction(mHost, mAction, mMode, mRec) {
 // alert('hostResources!');
  $("#tableHostPrint").html("<img class=\"system_image\" id=\"file_go\" src=\"images/loader.gif\"><h3>Обновление списка защищаемых ресурсов...</h3>");
  var xmlhttp = getXmlHttp(); // Создаём объект XMLHTTP
  xmlhttp.timeout = 30000;
  xmlhttp.ontimeout = function() {
    xmlhttp.abort();
    alert("Данные не получены от сервера (Timeout request).");
  }
  xmlhttp.open('POST', '/action', true); // Открываем асинхронное соединение
  xmlhttp.setRequestHeader('Cache-Control','max-age=3600');
  xmlhttp.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded'); 
  var Resp = "resourcebyhost=savelistmod";
  if(mHost.length)
    Resp += "&resourcehost=" + mHost; 
  if(mAction.length)
    Resp += "&resourcepath=" + mAction;
  if(mMode.length)
    Resp += "&resourcemode=" + mMode;  
  if(mRec.length && mRec == 'R')
    Resp += "&recursive=R";  
  xmlhttp.send(Resp); 
  xmlhttp.onreadystatechange = function() { // Ждём ответа от сервера
    if (xmlhttp.readyState == 4) { // Ответ пришёл        
      if(xmlhttp.status == 200) { // Сервер вернул код 200 (что хорошо)                    ;
		  var ListOfNames = JSON.parse(xmlhttp.responseText);		  
          if(ListOfNames.data.update == "OK"){
            $("#tableHostPrint").html("<img class=\"system_image\" id=\"file_go\" src=\"images/loader.gif\"><h3>Обновление данных страницы...</h3>");            
          }
          else{
            alert( "Не удалось обновить данные файла:" + mAction + " " + ListOfNames.data.update);                  
         }
         SaveResourceAction(mHost);
      }
      else if (xmlhttp.status == 401) { // Нет авторизации
        location.href = '/';
      }
      else {
        //обработка ошибки
        if(this.statusText.length)
          alert('error: ' + this.status ? this.statusText : 'query failed');
        return;
      }
    }
  };
}





function AuditAction(mHost, mAction, mConfig) {
  //alert('hostResources!');    
  $('.jnl','.ui-dialog').remove();
  $("#tableHostPrint").html("<img class=\"system_image\" id=\"file_go\" src=\"images/loader.gif\"><h3>Обновление списка аудита...</h3>")
  var xmlhttp = getXmlHttp(); // Создаём объект XMLHTTP
  xmlhttp.timeout = 60000;
  xmlhttp.ontimeout = function() {
    xmlhttp.abort();
    alert("Данные не получены от сервера (Timeout request).");
  }
  xmlhttp.open('POST', '/action', true); // Открываем асинхронное соединение
  xmlhttp.setRequestHeader('Cache-Control','max-age=3600');
  xmlhttp.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded');  
  var Resp = "resourcebyhost=auditlist";
  if(mAction.length)
    Resp += "&resourcepath=" + mAction;
  if(mHost.length)
    Resp += "&resourcehost=" + mHost; 
  //alert(Resp);
  xmlhttp.send(Resp);   
  xmlhttp.onreadystatechange = function() { // Ждём ответа от сервера
    if (xmlhttp.readyState == 4) { // Ответ пришёл        
      if(xmlhttp.status == 200) { // Сервер вернул код 200 (что хорошо)                    ;
          //alert(xmlhttp.responseText);
          $("#ResourceCurrent").html(""); 
          var url = mAction.substring(0, mAction.lastIndexOf('/'));
          url = url.substring(0, url.lastIndexOf('/'))
          
;         var out = "";
          var iDivObj = 0;
		  var ListOfNames = JSON.parse(xmlhttp.responseText);
          //alert(xmlhttp.responseText);
		  var NamesData   = ListOfNames.data;
          $("#tableHostPrint").html("");
          for(i in ListOfNames.data) {                                      
             iDivObj += 1;
          }                         
          if(iDivObj == 0) pathObj = "..";;                    
          $('#tableHostPrint').load('html/main_audit_tmp_files.html', '.main_audit_div_folder', function(){  
              
             ///XXX 
            var row_tmpl  = $(".main_audit_div_folder").find( $("tbody > tr.tmp_row_next")).html();
            $(".main_audit_div_folder").find( $("tbody > tr.tmp_row_next")).remove();
             for(i in ListOfNames.data) { 
              $('#pathtable_line tr:last').after("<tr>" + row_tmpl + "</tr>");
            }
              
              
            //все аудиты список                    ;
            LoadTemplateContentAuditFilesMain(mHost, mAction, ListOfNames, mConfig);                                        
            if(iDivObj == 0) pathObj = "..";        
           //загружаем описания аудитов
            if(mConfig != ""){          
              $('#pathtable_line').html("<tr></tr>")
              $('.auditblock_labels').load('html/audit_tmp_rules.html', '.rules_audit_header', function(){  
                var block  =  $(".auditblock_labels").html();  
                if(iDivObj > 0){
                  LoadTemplateContentAuditList(mHost, mAction, ListOfNames.data[0].rules,block);
                }
                else{
                  LoadTemplateContentAuditList(mHost, mAction, "", block);
                }
              });                                             
            }
            else{
              $('.auditblock_labels').load('html/audit_tmp_files.html', '.rules_audit_div_folder', function(){  
                LoadTemplateContentAuditFiles(mHost, mAction,'');
              });  
            }
            
            $(".AuditMainSource").find( $("tbody > tr.device_row_")).remove();
            var table = $('.AuditMainSource').DataTable();
            
         });
          if(mAction == "/") mAction = "";
        document.getElementById("ResourceCurrent").innerHTML = "<span>Аудит ресурсов: <p class=\"resource_host\">" + mHost + "</p><p class=\"resource_path\">" +  mAction + "</p></span>";       
         // }            

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


function AuditActionAdd(mHost, mAction, mAuditRule, RecursiveFlag, mConfig) {
 // alert('hostResources!');
  var xmlhttp = getXmlHttp(); // Создаём объект XMLHTTP
  xmlhttp.timeout = 60000;
  xmlhttp.ontimeout = function() {
    xmlhttp.abort();
    alert("Данные не получены от сервера (Timeout request).");
  }
  xmlhttp.open('POST', '/action', true); // Открываем асинхронное соединение
  xmlhttp.setRequestHeader('Cache-Control','max-age=3600');
  xmlhttp.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded'); 
  var Resp = "resourcebyhost=auditmodify";
  if(mHost.length)
    Resp += "&resourcehost=" + mHost; 
  else return;
  if(mAction.length)
    Resp += "&resourcepath=" + mAction;
  else return;
  if(mAuditRule.length)
    Resp += "&auditrule=" + mAuditRule;  
  else return;
  if(RecursiveFlag == true)
    Resp += "&attrrec=-R"; 
  xmlhttp.send(Resp); 
  xmlhttp.onreadystatechange = function() { // Ждём ответа от сервера
    if (xmlhttp.readyState == 4) { // Ответ пришёл        
      if(xmlhttp.status == 200) { // Сервер вернул код 200 (что хорошо)                    ;
		  var ListOfNames = JSON.parse(xmlhttp.responseText);		  
          if(ListOfNames.data.update == "OK"){
            if(mAuditRule == "-"  &&  mConfig == ""){
              $("#tableHostPrint").html("<img class=\"system_image\" id=\"file_go\" src=\"images/loader.gif\"><h3>Обновление списка аудита...</h3>")
              AuditAction(mHost,"/","");
            }
            else
              AuditAction(mHost, mAction, mConfig);
            
          }
          else{
            alert( "Не удалось обновить атрибуты файла:" + mActio + " " + ListOfNames.data.update);                  
         }

      }
      else if (xmlhttp.status == 401) { // Нет авторизации
        location.href = '/';
      }
      else {
        //обработка ошибки
        if(this.statusText.length)
          alert('error: ' + this.status ? this.statusText : 'query failed');
        return;
      }
    }
  };
}



function SaveSettingsAction() {
  //alert('hostResources!'); 
  var xmlhttp = getXmlHttp(); // Создаём объект XMLHTTP
  xmlhttp.timeout = 60000;
  xmlhttp.ontimeout = function() {
    xmlhttp.abort();
    alert("Данные не получены от сервера (Timeout request).");
  }
  xmlhttp.open('POST', '/action', true); // Открываем асинхронное соединение
  xmlhttp.setRequestHeader('Cache-Control','max-age=3600');
  xmlhttp.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded'); 
  var Resp = "resourcebyhost=savesettings";
  xmlhttp.send(Resp); 
  xmlhttp.onreadystatechange = function() { // Ждём ответа от сервера
    if (xmlhttp.readyState == 4) { // Ответ пришёл        
      if(xmlhttp.status == 200) { // Сервер вернул код 200 (что хорошо)                    ;
		  var ListOfNames = JSON.parse(xmlhttp.responseText);		  
          //alert(xmlhttp.responseText);            
          if(ListOfNames.data.ftp_server != undefined);
            $('#_server').val(ListOfNames.data.ftp_server);
          if(ListOfNames.data.ftp_user != undefined);
            $('#_login').val(ListOfNames.data.ftp_user);
          if(ListOfNames.data.ftp_pass != undefined);
            $('#_password').val(ListOfNames.data.ftp_pass);
            $("#_server").focus();
                
          /*if(ListOfNames.data.ftp_server == undefined || !ListOfNames.data.ftp_server.length){
            alert('Не заданы настройки сервера FTP');
          }*/
          
      }
      else if (xmlhttp.status == 401) { // Нет авторизации
        location.href = '/';
      }
      else {
        //обработка ошибки
        if(this.statusText.length)
          alert('error: ' + this.status ? this.statusText : 'query failed');
        return;
      }
    }
  };
}


function UpdateSaveSettingsAction(mServer, mUser, mPass) {
  //alert('hostResources!'); 
  var xmlhttp = getXmlHttp(); // Создаём объект XMLHTTP
  xmlhttp.timeout = 60000;
  xmlhttp.ontimeout = function() {
    xmlhttp.abort();
    alert("Данные не получены от сервера (Timeout request).");
  }
  xmlhttp.open('POST', '/action', true); // Открываем асинхронное соединение
  xmlhttp.setRequestHeader('Cache-Control','max-age=3600');
  xmlhttp.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded'); 
  var Resp = "resourcebyhost=replacesavesettings";
  if(mServer.length)
    Resp += "&ftp_server=" + mServer;
  if(mUser.length)
    Resp += "&ftp_user=" + mUser;
  if(mPass.length)
    Resp += "&ftp_pass=" + mPass;
  xmlhttp.send(Resp); 
  xmlhttp.onreadystatechange = function() { // Ждём ответа от сервера
    if (xmlhttp.readyState == 4) { // Ответ пришёл        
      if(xmlhttp.status == 200) { // Сервер вернул код 200 (что хорошо)                    ;
		  var ListOfNames = JSON.parse(xmlhttp.responseText);		  
          //alert(xmlhttp.responseText);            
          if(ListOfNames.data.update == "OK"){
            SaveSettingsAction();
            $('#_server_save').dialog('close');
          }
          
      }
      else if (xmlhttp.status == 401) { // Нет авторизации
        location.href = '/';
      }
      else {
        //обработка ошибки
        if(this.statusText.length)
          alert('error: ' + this.status ? this.statusText : 'query failed');
        return;
      }
    }
  };
}




function AuditJournalAction(mHost, mAction) {
  //alert('hostResources!');       
  $(".jnl").html("<img class=\"system_image\" id=\"file_go\" src=\"images/loader.gif\"><h3>Загрузка данных журнала аудита...</h3>");
  document.getElementById('table_save_audit_folder').style.display = "none";
 
  var xmlhttp = getXmlHttp(); // Создаём объект XMLHTTP
  xmlhttp.timeout = 60000;
  xmlhttp.ontimeout = function() {
    xmlhttp.abort();
    alert("Данные не получены от сервера (Timeout request).");
  }
  xmlhttp.open('POST', '/action', true); // Открываем асинхронное соединение
  xmlhttp.setRequestHeader('Cache-Control','max-age=3600');
  xmlhttp.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded'); 
  var Resp = "resourcebyhost=getauditjournal";
  if(mHost.length)
    Resp += "&resourcehost=" + mHost; 
  else return;
  if(mAction.length)
    Resp += "&resourcepath=" + mAction;
  else return; 
  xmlhttp.send(Resp); 
  xmlhttp.onreadystatechange = function() { // Ждём ответа от сервера
    if (xmlhttp.readyState == 4) { // Ответ пришёл        
      if(xmlhttp.status == 200) { // Сервер вернул код 200 (что хорошо)                    ;
		  var ListOfNames = JSON.parse(xmlhttp.responseText);		  
          //alert(xmlhttp.responseText);            
          $('.jnl').html("<h3>Журнал аудита ресурса:<\h3><span><p>"  + mAction + "</p></span>").ready(function () {
            for(i in ListOfNames.data) { 
              $('.jnl').append("<span><p>"  + ListOfNames.data[i] + "</p></span>");    
            }               
         });         
         $('.jnl').dialog({ 
                           width: 500 ,
                           height: 500 });
      }
      else if (xmlhttp.status == 401) { // Нет авторизации
        location.href = '/';
      }
      else {
        //обработка ошибки
        if(this.statusText.length)
          alert('error: ' + this.status ? this.statusText : 'query failed');
        return;
      }
    }
  };
}




function ResourceAttribute(mHost, mAction, mPage, mFile) {
  $("#tableHostPrint").html("<img class=\"system_image\" id=\"file_go\" src=\"images/loader.gif\"><h3>Обновление данных атрибутов...</h3>");
  var xmlhttp = getXmlHttp(); // Создаём объект XMLHTTP
  xmlhttp.timeout = 60000;
  xmlhttp.ontimeout = function() {
    xmlhttp.abort();
    alert("Данные не получены от сервера (Timeout request).");
  }
  xmlhttp.open('POST', '/action', true); // Открываем асинхронное соединение
  xmlhttp.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded');  
  if(mAction.length)
    xmlhttp.send("resourcebyhost=getaclattr&resourcehost=" + mHost + "&resourcepath=" + mAction); 
  else
    xmlhttp.send("resourcebyhost=getaclattr&resourcehost=" + mHost);   
  xmlhttp.onreadystatechange = function() { // Ждём ответа от сервера
    if (xmlhttp.readyState == 4) { // Ответ пришёл        
      if(xmlhttp.status == 200) { // Сервер вернул код 200 (что хорошо)                    
          //alert(xmlhttp.responseText);                   
          var url = mAction.substring(0, mAction.lastIndexOf('/'));
          url = url.substring(0, url.lastIndexOf('/'))
          
;         var out = "";
          var iDivObj = 0;
		  var ListOfNames = JSON.parse(xmlhttp.responseText);
          //alert(xmlhttp.responseText);
          var NamesFile   = ListOfNames.data.acl_attributes.res_hame;
          var NamesOwner  = ListOfNames.data.acl_attributes.res_owner;
          var NamesGroup  = ListOfNames.data.acl_attributes.res_group;
          var NamesFlags  = ListOfNames.data.acl_attributes.res_flags;
          
          var PermOwner  = "---";
          var PermGroup  = "---";
          var PermOther  = "---";
          var AclObjects = "";
          var DefAclObjectUser      = "";
          var DefAclObjectUserIcon  = "";
          var DefAclObjectGroup      = "";
          var DefAclObjectGroupIcon  = "";
          var DefAclObjectOther      = "";
          var DefAclObjectOtherIcon  = "";
          var AclCnt = 0;          
          var v_def_user = 0;
          var v_def_group = 0;
          var v_def_other = 0;
          
          out += "<span id=\"glowtext\" onclick=\"CheckLoader(this);hostAction(\'" + mHost + "\',\'" + mAction.substring(0, mAction.lastIndexOf('/')) + "/\');\"><img class=\"system_image\" id=\"back_go\"  src=\"images/back.png\">Вернуться к списку файлов и директорий</span><br>";          
          
          $('#tableHostPrint').html("");
           $('#tableHostPrint').load('html/discrete_header_block.html', '.discrete_list_all_div', function(){
               
           if( mFile != undefined){
             $('#filetype_').attr('value', mFile);  
           }
           var file_acl_tmpl  = $(".table_cur_acl_rules").find( $("tbody > tr.def_row_next")).html(); 
           $(".table_cur_acl_rules").find( $("tbody > tr.def_row_next")).remove();          
             
          for(i in ListOfNames.data.acl_attributes.acl_node) {  
            var pathObj;
            var pathFull;  
            var type_acl_par = "";
            var type_acl = ListOfNames.data.acl_attributes.acl_node[i].res_obj;
            //Rules OUNER USER GROUP discrete
            if(ListOfNames.data.acl_attributes.acl_node[i].res_obj == "user"){
              type_acl = "Пользователь";
              type_acl_par += "u"; 
              if(ListOfNames.data.acl_attributes.acl_node[i].res_what == "")
                PermOwner = ListOfNames.data.acl_attributes.acl_node[i].res_mask; 
            }
            if(ListOfNames.data.acl_attributes.acl_node[i].res_obj == "group"){
              type_acl = "Группа";
              type_acl_par += "g";
              if(ListOfNames.data.acl_attributes.acl_node[i].res_what == "")
                PermGroup = ListOfNames.data.acl_attributes.acl_node[i].res_mask;  
            }
            if(ListOfNames.data.acl_attributes.acl_node[i].res_obj == "other"){
             type_acl = "Остальные";
             type_acl_par += "o";
             if(ListOfNames.data.acl_attributes.acl_node[i].res_what == "")
               PermOther = ListOfNames.data.acl_attributes.acl_node[i].res_mask; 
           }
            if(ListOfNames.data.acl_attributes.acl_node[i].res_obj == "mask"){
              type_acl = "Маска";
              type_acl_par += "m";               
           }   
           //ACL rules for groups and users
           if(ListOfNames.data.acl_attributes.acl_node[i].res_what != "" ||
               ListOfNames.data.acl_attributes.acl_node[i].res_obj == "mask"){   
              $('.table_cur_acl_rules tr:last').after("<tr>" + file_acl_tmpl + "</tr>");
              HTMLACLRules(mHost, mAction, type_acl, type_acl_par,ListOfNames.data.acl_attributes.acl_node[i]);
              AclCnt++;
           }    
            //default rules for OWNER USER GROUP
           if(ListOfNames.data.acl_attributes.acl_node[i].res_what == "" && 
                ListOfNames.data.acl_attributes.acl_node[i].default != undefined &&
                (type_acl_par == "u" || type_acl_par == "g" || type_acl_par == "o")){                                              ;
                if(type_acl_par == "u"){
                  HTMLDefineBlock(type_acl_par, ListOfNames.data.acl_attributes.acl_node[i].default);
                  v_def_user = v_def_user + 1;
                }
                else if(type_acl_par == "g"){
                  HTMLDefineBlock(type_acl_par, ListOfNames.data.acl_attributes.acl_node[i].default);
                  v_def_group = v_def_group + 1;
                }
                else if(type_acl_par == "o"){
                  HTMLDefineBlock(type_acl_par, ListOfNames.data.acl_attributes.acl_node[i].default);
                  v_def_other = v_def_other + 1;
                }
                                    
            }
            iDivObj += 1;
          } 
          
          
          if(iDivObj > 0){                
            var rec_check = (g_checked_rec == true)?"checked":"";
            var PermAll = "";
            PermAll += PermOwner + PermGroup + PermOther;   
            if(v_def_user == 0)  $('.DefAclObjectUser').html("");
            if(v_def_group == 0) $('.DefAclObjectGroup').html("");
            if(v_def_other == 0) $('.DefAclObjectOther').html("");
           /*Load blocks templates*/                                                    
                                    
                $('.header_list').html(out + "<div id=\"main_rights_block\"><span><p id=\"controls_rec\"><input type=\"checkbox\" id=\"s_recursive\" value=\"recursive\" " + rec_check + "> Применить рекурсию</p></span></div>");
                /*Load data in blocks*/
                 DiscreteBitsHtmlBlock(NamesOwner, NamesGroup, NamesFlags);
                 /**/
                 DiscreteHtmlBlock(mHost, mAction, PermAll);
                 /**/
                 ACLHtmlBlock(mHost, mAction);
                if(mPage == 2){                    
                     $('#w_2_tab_descr').addClass('active').siblings().removeClass('active').closest('div.tabs_discr').find('div.tabs__content_discr').removeClass('active').eq($('#w_2_tab_descr').index()).addClass('active');
                 }
                 ResourceMandatObjects(mHost, mAction); 
                                 
            /*Fill params settings / selectors*/        
            $('#ResourceCurrent').html(mHost + "  " +  NamesFile);  
            ResourceLocalObjectsSelect(mHost,"Users","#audit_rule_who_id_" ,0, 0); 
            
            ResourceLocalObjectsSelectNamed(mHost,'Groups','#discrete_who_id_own',0,0, NamesOwner);
            ResourceLocalObjectsSelectNamed(mHost,'Users','#discrete_who_id_group',0,0, NamesGroup);
                         
            UpdateRulesEditForm ();
          }
                     
        });  

      }
      else if (xmlhttp.status == 401) { // Нет авторизации
        location.href = '/';
      }
      else {
        //обработка ошибки
        if(this.statusText.length)
          alert('error: ' + this.status ? this.statusText : 'query failed');
        return;
      }
    }
  };
}



function ResourceDiscrete(mHost, mAction, mValueOld, mValue, mRec, mBits, mChown_own_new, mChown_group_new, mChown_own_old, mChown_group_old) {
  //alert('AttributeDiscrete!');
  var xmlhttp = getXmlHttp(); // Создаём объект XMLHTTP
  xmlhttp.timeout = 60000;
  xmlhttp.ontimeout = function() {
    xmlhttp.abort();
    alert("Данные не получены от сервера (Timeout request).");
  }
  xmlhttp.open('POST', '/action', true); // Открываем асинхронное соединение
  xmlhttp.setRequestHeader('Cache-Control','max-age=3600');
  xmlhttp.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded');  
  if(!mAction.length) return;
  if(!mValue.length)  return;
  
  
  var HttpDataParams = "resourcebyhost=setdiscrete&resourcehost=" + mHost + "&resourcepath=" + mAction + "&attrrec=" + mRec;  
  if(mValue != mValueOld){
    HttpDataParams += "&attrvalue=" + mValue;    
  }
  if(mBits != 0){ 
    if(mBits&(1<<0)){
      var bValue = ($('#s_bit_suid').is(':checked'))?"1":"0";  
      HttpDataParams += "&s_bit_suid=" + bValue;
    }
    if(mBits&(1<<1)){
      var bValue = ($('#s_bit_sguid').is(':checked'))?"1":"0";  
      HttpDataParams += "&s_bit_sguid=" + bValue;
    }
    if(mBits&(1<<2)){
      var bValue = ($('#s_bit_sticky').is(':checked'))?"1":"0";  
      HttpDataParams += "&s_bit_sticky=" + bValue;
    }      
  }
  
  if(mChown_own_new   != mChown_own_old ||
     mChown_group_new != mChown_group_old){
    HttpDataParams += "&chown_owner=" + mChown_own_new;     
    HttpDataParams += "&chown_group=" + mChown_group_new;      
  }

  xmlhttp.send(HttpDataParams);       
  xmlhttp.onreadystatechange = function() { // Ждём ответа от сервера
    if (xmlhttp.readyState == 4) { // Ответ пришёл        
      if(xmlhttp.status == 200) { // Сервер вернул код 200 (что хорошо)                    
          var ListOfNames = JSON.parse(xmlhttp.responseText);
		  var NamesData   = ListOfNames.data;
          if(ListOfNames.data.update == "OK"){                            
            var prop_mode = $('#filetype_').attr('value');
            ResourceAttribute(mHost, mAction, 1, prop_mode); 
          }
          else{
            alert( "Не удалось обновить атрибуты файла:" + mActio + " " + ListOfNames.data.update);                  
         }
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

function ResourceLocalObjectsSelect(mHost, mType, mSelect, mDomain, Filter) {
  var HttpDataParams = "resourcehost=" + mHost;
  var xmlhttp = getXmlHttp(); // Создаём объект XMLHTTP
  xmlhttp.timeout = 60000;
  xmlhttp.ontimeout = function() {
    xmlhttp.abort();
    alert("Данные не получены от сервера (Timeout request).");
  }
  if(mDomain == 1)
    xmlhttp.open('POST', '/ald', true); // Открываем асинхронное соединение
  else
    xmlhttp.open('POST', '/action', true); // Открываем асинхронное соединение
  xmlhttp.setRequestHeader('Cache-Control','max-age=3600');
  xmlhttp.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded');  
  
  if(mType == "Users"){
    if(mDomain == 1)
      HttpDataParams += "&userRawAld=userlist";
    else 
      HttpDataParams += "&resourcebyhost=localonlyuserlist";
  }
  else if(mType == "Groups"){
    if(mDomain == 1)
      HttpDataParams += "&groupRawAld=grouplist";
    else 
      HttpDataParams += "&resourcebyhost=localonlygrouplist";
  }
  if(Filter == 1) HttpDataParams +=  "&domainfilter=1";
 //alert(HttpDataParams);
  xmlhttp.send(HttpDataParams);       
  xmlhttp.onreadystatechange = function() { // Ждём ответа от сервера
    if (xmlhttp.readyState == 4) { // Ответ пришёл        
      if(xmlhttp.status == 200) { // Сервер вернул код 200 (что хорошо)                    
          //alert(xmlhttp.responseText);  
          var ListOfNames = JSON.parse(xmlhttp.responseText);		            
          $(mSelect).html("");                    
          for(i in ListOfNames.data) {  
            var NamesData   = ListOfNames.data[i];      
            $(mSelect).append("<option value=\"" + NamesData +"\">"+ NamesData +"</option>").ready(function () {      
            });                        
         }
      }
      else if (xmlhttp.status == 401) { // Нет авторизации
        location.href = '/';
      }
      else {
        //обработка ошибки
        if(this.statusText.length)
          alert('error: ' + this.status ? this.statusText : 'query failed');
        return;
      }
    }
  };
}



function ResourceLocalObjectsSelectNamed(mHost, mType, mSelect, mDomain, Filter, mName) {
  var HttpDataParams = "resourcehost=" + mHost;
  var xmlhttp = getXmlHttp(); // Создаём объект XMLHTTP
  xmlhttp.timeout = 60000;
  xmlhttp.ontimeout = function() {
    xmlhttp.abort();
    alert("Данные не получены от сервера (Timeout request).");
  }
  if(mDomain == 1)
    xmlhttp.open('POST', '/ald', true); // Открываем асинхронное соединение
  else
    xmlhttp.open('POST', '/action', true); // Открываем асинхронное соединение
  xmlhttp.setRequestHeader('Cache-Control','max-age=3600');
  xmlhttp.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded');  
  
  if(mType == "Users"){
    if(mDomain == 1)
      HttpDataParams += "&userRawAld=userlist";
    else 
      HttpDataParams += "&resourcebyhost=localonlyuserlist";
  }
  else if(mType == "Groups"){
    if(mDomain == 1)
      HttpDataParams += "&groupRawAld=grouplist";
    else 
      HttpDataParams += "&resourcebyhost=localonlygrouplist";
  }
  if(Filter == 1) HttpDataParams +=  "&domainfilter=1";
 //alert(HttpDataParams);
  xmlhttp.send(HttpDataParams);       
  xmlhttp.onreadystatechange = function() { // Ждём ответа от сервера
    if (xmlhttp.readyState == 4) { // Ответ пришёл        
      if(xmlhttp.status == 200) { // Сервер вернул код 200 (что хорошо)                    
          //alert(xmlhttp.responseText);  
          var ListOfNames = JSON.parse(xmlhttp.responseText);		            
          $(mSelect).html("");                    
          for(i in ListOfNames.data) {  
            var NamesData   = ListOfNames.data[i];      
            $(mSelect).append("<option value=\"" + NamesData +"\">"+ NamesData +"</option>").ready(function () {      
            });                        
         }         
          $(mSelect).val(mName);
          $(mSelect).attr('save', mName);
 
      }
      else if (xmlhttp.status == 401) { // Нет авторизации
        location.href = '/';
      }
      else {
        //обработка ошибки
        if(this.statusText.length)
          alert('error: ' + this.status ? this.statusText : 'query failed');
        return;
      }
    }
  };
}

function ResourceMandatObjects(mHost, mAction) {
  //alert('AttributeMandat!');
  $("#MandatObjects .tabs__content_discr").html("<img class=\"system_image\" id=\"file_go\"  src=\"images/loader.gif\"><h3>Обновление атрибутов контроля целостности....</h3>");
  var xmlhttp = getXmlHttp(); // Создаём объект XMLHTTP
  xmlhttp.timeout = 60000;
  xmlhttp.ontimeout = function() {
    xmlhttp.abort();
    alert("Данные не получены от сервера (Timeout request).");
  }
  xmlhttp.open('POST', '/action', true); // Открываем асинхронное соединение
  xmlhttp.setRequestHeader('Cache-Control','max-age=3600');
  xmlhttp.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded');  
  var HttpDataParams = "resourcebyhost=mandatlist";
  if(!mHost.length) return;
  if(!mAction.length) return;    
  HttpDataParams += "&resourcehost=" + mHost;
  HttpDataParams += "&resourcepath=" + mAction;
  xmlhttp.send(HttpDataParams);       
  xmlhttp.onreadystatechange = function() { // Ждём ответа от сервера
    if (xmlhttp.readyState == 4) { // Ответ пришёл        
      if(xmlhttp.status == 200) { // Сервер вернул код 200 (что хорошо)                    
          //alert(xmlhttp.responseText);  
          var ListOfNames = JSON.parse(xmlhttp.responseText);		  
          MandatHtmlBlock(mHost, mAction, ListOfNames);                                      
      }
      else if (xmlhttp.status == 401) { // Нет авторизации
        location.href = '/';
      }
      else {
        //обработка ошибки
        if(this.statusText.length)
          alert('error: ' + this.status ? this.statusText : 'query failed');
        return;
      }
    }
  };
}


function ResourceMandatObjectsSave(mHost, mAction) {
  //alert('AttributeMandatSave!');
  var xmlhttp = getXmlHttp(); // Создаём объект XMLHTTP
  xmlhttp.timeout = 60000;
  xmlhttp.ontimeout = function() {
    xmlhttp.abort();
    alert("Данные не получены от сервера (Timeout request).");
  }
  xmlhttp.open('POST', '/action', true); // Открываем асинхронное соединение
  xmlhttp.setRequestHeader('Cache-Control','max-age=3600');
  xmlhttp.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded');  
  var HttpDataParams = "resourcebyhost=setmandatrule";
  if ($('#s_recursive').is(':checked'))
    HttpDataParams += "&attrrec=-Rv";
  if(!mHost.length) return;
  if(!mAction.length) return;    
  HttpDataParams += "&resourcehost=" + mHost;
  HttpDataParams += "&resourcepath=" + mAction;
  var mandatrule = "";
  var selectWho;
  selectWho = document.getElementById("MandatObjectLevals"); 
  var mObjLevelSecurity = selectWho.options[selectWho.selectedIndex].value;
  selectWho = document.getElementById("MandatObjectMonolite"); 
  var mObjLevelMonolite = selectWho.options[selectWho.selectedIndex].text; 
  var bitsFlags = 0x00;
  if(document.getElementById("flag_ccnr").checked == true)            
    bitsFlags |= (1<<0);
  if(document.getElementById("flag_ccnri").checked == true)            
    bitsFlags |= (1<<1);
  if(document.getElementById("flag_ehole").checked == true)            
    bitsFlags |= (1<<2);
  if(document.getElementById("flag_whole").checked == true)            
    bitsFlags |= (1<<3);
  var MaskFlags = [0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00];  
  var posBits = 0;
  for(var i = 0; i < 8; i++) { 
    for(var e = 0; e < 8; e++){
      var CheckVal = document.getElementById("flag_categ_" + posBits);
      if(CheckVal != undefined){
        if (document.getElementById("flag_categ_" + posBits).checked == true) { 
            MaskFlags[i] |= (1<<e);            
        }            
      } 
      posBits++;
    }      
  }
  var i = 8;
  var StrHexAll = "";
  while(i--){ 
    var StrByte = MaskFlags[i].toString(16);
    StrHexAll += (StrByte.length < 2)?("0" + StrByte):StrByte;
  }
  var StrByteBits = bitsFlags.toString(16);
  var StrByteHex  = (StrByteBits.length < 2)?("0x0" + StrByteBits):("0x" + StrByteBits);
  mandatrule += mObjLevelSecurity + ":" + mObjLevelMonolite + ":0x" + StrHexAll  + ":" + StrByteHex;
  HttpDataParams += "&mandatrule="   + mandatrule;
  xmlhttp.send(HttpDataParams);       
  xmlhttp.onreadystatechange = function() { // Ждём ответа от сервера
    if (xmlhttp.readyState == 4) { // Ответ пришёл        
      if(xmlhttp.status == 200) { // Сервер вернул код 200 (что хорошо)                    
          //alert(xmlhttp.responseText);  
          var ListOfNames = JSON.parse(xmlhttp.responseText);		  
          if(ListOfNames.data.update == "OK"){  
            ResourceMandatObjects(mHost, mAction);
          }
          else 
            alert("error: " + ListOfNames.data.update)

      }
      else if (xmlhttp.status == 401) { // Нет авторизации
        location.href = '/';
      }
      else {
        //обработка ошибки
        if(this.statusText.length)
          alert('error: ' + this.status ? this.statusText : 'query failed');
        return;
      }
    }
  };
}


function ResourceDeleteAcl(mHost, mAction, mAcl, mAll) {
  //alert('ResourceDeleteAcl!');
  var xmlhttp = getXmlHttp(); // Создаём объект XMLHTTP
  xmlhttp.timeout = 60000;
  xmlhttp.ontimeout = function() {
    xmlhttp.abort();
    alert("Данные не получены от сервера (Timeout request).");
  }
  xmlhttp.open('POST', '/action', true); // Открываем асинхронное соединение
  xmlhttp.setRequestHeader('Cache-Control','max-age=3600');
  xmlhttp.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded');  
  var HttpDataParams = "";
  if(mAll == 1)
   HttpDataParams = "resourcebyhost=clearacllist";
  else
   HttpDataParams = "resourcebyhost=deleteacllist";
  HttpDataParams += "&resourcehost=" + mHost;
  if(!mAction.length)
    return;
  else
    HttpDataParams += "&resourcepath=" + mAction;
  if(mAcl.length)
    HttpDataParams += "&resourcerule=" + mAcl;  
  if ($('#s_recursive').is(':checked'))
    HttpDataParams += "&attrrec=-R";
      
  xmlhttp.send(HttpDataParams);    
  xmlhttp.onreadystatechange = function() { // Ждём ответа от сервера
    if (xmlhttp.readyState == 4) { // Ответ пришёл        
      if(xmlhttp.status == 200) { // Сервер вернул код 200 (что хорошо)                    
          //alert(xmlhttp.responseText);  
          var ListOfNames = JSON.parse(xmlhttp.responseText);
		  var NamesData   = ListOfNames.data;
          if(ListOfNames.data.update == "OK"){
            var prop_mode = $('#filetype_').attr('value');
            ResourceAttribute(mHost, mAction, 2, prop_mode); 
          }
          else{
            alert( "Не удалось обновить атрибуты файла:" + mAction + " " + ListOfNames.data.update);                  
         }
      }
      else if (xmlhttp.status == 401) { // Нет авторизации
        location.href = '/';
      }
      else {
        //обработка ошибки
        if(this.statusText.length)
          alert('error: ' + this.status ? this.statusText : 'query failed');
        return;
      }
    }
  };
}


function ResourceAddAcl(mHost, mAction) {
  //alert('AddAcl!');
  var xmlhttp = getXmlHttp(); // Создаём объект XMLHTTP
  xmlhttp.timeout = 36000;
  xmlhttp.ontimeout = function() {
    xmlhttp.abort();
    alert("Данные не получены от сервера (Timeout request).");
  }
  xmlhttp.open('POST', '/action', true); // Открываем асинхронное соединение
  xmlhttp.setRequestHeader('Cache-Control','max-age=3600');
  xmlhttp.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded');  
  var HttpDataParams = "";
  var mAcl = "";
  HttpDataParams = "resourcebyhost=addacllist";
  HttpDataParams += "&resourcehost=" + mHost;
  if(!mAction.length)
    return;
  else
    HttpDataParams += "&resourcepath=" + mAction;
  var mAclRule  = "";
  var mAclRWX   = "";
  var mAclObj   = "";
  var mAclTypeRule = "";
  var mAclTypeDef  = "";
  var a_bit_r   =  (document.getElementById("acl_bit_r").checked)?"r":"-";
  var a_bit_w   =  (document.getElementById("acl_bit_w").checked)?"w":"-";
  var a_bit_e   =  (document.getElementById("acl_bit_e").checked)?"x":"-";
  mAclRWX += ":" + a_bit_r + a_bit_w + a_bit_e;
  
  var inpType = document.getElementsByName('dradio_type');
    for (var i = 0; i < inpType.length; i++) {
        if (inpType[i].type == "radio" && inpType[i].checked) {
            mAclTypeRule  = inpType[i].value;            
            if((mAclTypeRule == "u:" || mAclTypeRule == "g:")){
              var selectWho = document.getElementById("audit_rule_who_id_"); 
              mAclObj = selectWho.options[selectWho.selectedIndex].text; 
            }                           
        }
    }
  if(document.getElementById("acl_def_bit").checked)
    mAclTypeDef = "default:";    
  
  mAclRule = mAclTypeDef + mAclTypeRule + mAclObj +  mAclRWX;  
  HttpDataParams += "&resourcerule=" + mAclRule;    
  if ($('#s_recursive').is(':checked'))
    HttpDataParams += "&attrrec=-R";
  //alert(HttpDataParams);
  xmlhttp.send(HttpDataParams);    
  xmlhttp.onreadystatechange = function() { // Ждём ответа от сервера
    if (xmlhttp.readyState == 4) { // Ответ пришёл        
      if(xmlhttp.status == 200) { // Сервер вернул код 200 (что хорошо)                    
          //alert(xmlhttp.responseText);  
          var ListOfNames = JSON.parse(xmlhttp.responseText);
		  var NamesData   = ListOfNames.data;
          if(ListOfNames.data.update == "OK"){
            var prop_mode = $('#filetype_').attr('value');
            ResourceAttribute(mHost, mAction, 2, prop_mode);
          }
          else{
            alert( "Не удалось обновить атрибуты файла:" + mAction + " " + ListOfNames.data.update);                  
         }
      }
      else if (xmlhttp.status == 401) { // Нет авторизации
        location.href = '/';
      }
      else {
        //обработка ошибки
        if(this.statusText.length)
          alert('error: ' + this.status ? this.statusText : 'query failed');
        return;
      }
    }
  };
}


function take_path(mHost, pathObj){
   //alert(mHost); 
   //alert(pathObj);
    
}

function HTMLACLRules(mHost, mAction, type_acl, type_acl_par, mListAlsObj){
  
  var mWhat = mListAlsObj.res_what; 
  var type_acl = type_acl_par +  ":" + mWhat;
  var type_ac_who ="";
  if(type_acl_par == "u") type_ac_who = "Пользователь";
  if(type_acl_par == "g") type_ac_who = "Группа";
  if(type_acl_par == "o") type_ac_who = "Остальные";
  if(type_acl_par == "m") type_ac_who = "Маска";

  $('.table_cur_acl_rules tr:last .acl_td_1').text(type_ac_who);
  if(mWhat == "")
      mWhat = "-";
  $('.table_cur_acl_rules tr:last .acl_td_2').html(mWhat);
  $('.table_cur_acl_rules tr:last .acl_td_3').html(mListAlsObj.res_mask);
  $('.table_cur_acl_rules tr:last .acl_td_4').html("<span id=\"glowtext\" onclick=\"ResourceDeleteAcl(\'" + mHost + "\',\'" + mAction + "\',\'" + type_acl + "\',\'0\');\"><img class=\"system_image\" src=\"images/delete_icon.png\"></span>");            
  if(mListAlsObj.default != undefined){                                  
    var defObj  = "default";                        
    defObj  += ":" + mListAlsObj.default.res_obj;
    defObj  += ":" + mListAlsObj.default.res_what;    
    $('.table_cur_acl_rules tr:last .acl_td_5').html(mListAlsObj.default.res_mask);
    $('.table_cur_acl_rules tr:last .acl_td_6').html("<span id=\"glowtext\" onclick=\"ResourceDeleteAcl(\'" + mHost + "\',\'" + mAction + "\',\'" + defObj + "\',\'0\');\"><img class=\"system_image\" src=\"images/delete_icon.png\"></span>");
  }
}

function  HTMLDefineBlock(type_acl_par, ListDefaults) {
    $(document.getElementById("def_" + type_acl_par + "_1")).prop('checked', (ListDefaults.res_mask[0] == "r")?true:false);     
    $(document.getElementById("def_" + type_acl_par + "_2")).prop('checked', (ListDefaults.res_mask[1] == "w")?true:false);    
    $(document.getElementById("def_" + type_acl_par + "_3")).prop('checked', (ListDefaults.res_mask[2] == "x")?true:false);
            
}
                    
function DiscreteBitsHtmlBlock(NamesOwner, NamesGroup, NamesFlags){                    
  var BitsBlock  = "";
  var bit_suid   = false;
  var bit_sguid  = false;
  var bit_sticky = false;
  
  var host =  $('.resource_host').text();
                          
  if(NamesFlags[0] == "s") bit_suid = "checked";
  if(NamesFlags[1] == "s") bit_sguid = "checked";
  if(NamesFlags[2] == "t") bit_sticky = "checked";
  
   $('#s_bit_suid').prop('value', bit_suid);
   $('#s_bit_sguid').prop('value', bit_sguid);
   $('#s_bit_sticky').prop('value', bit_sticky);
   
   $('#s_bit_suid').prop('checked', ((NamesFlags[0] == "s")?true:false));
   $('#s_bit_sguid').prop('checked', ((NamesFlags[1] == "s")?true:false));
   $('#s_bit_sticky').prop('checked', ((NamesFlags[2] == "t")?true:false));
              
   //$(".NamesOwner").text(NamesOwner);
   //$('#discrete_who_id_own').val(NamesOwner);
   //$(".NamesGroup").text(NamesGroup);
   //$('#discrete_who_id_group').val(NamesGroup);
}            

function DiscreteHtmlBlock(mHost, mAction, PermAll){
  var DiscreteBlock  = "";
  var AclDiscreteVal = 0;
  var bit_owner_r = "";
  var bit_owner_w  = "";
  var bit_owner_e = ""; 
             
  var bit_group_r = "";
  var bit_group_w  = "";
  var bit_group_e = "";
             
  var bit_other_r = "";
  var bit_other_w  = "";
  var bit_other_e = "";
             
  var PermMaks = "rwxrwxrwx";             
  for(var k = 0; k < 9; k++){ 
    if(PermAll[k] == PermMaks[k]) AclDiscreteVal|=(1<<(8 - k));
  }
  var octVal =   (AclDiscreteVal).toString(8); 
  if(PermAll[0] == "r") bit_owner_r = "checked";
  if(PermAll[1] == "w") bit_owner_w = "checked";
  if(PermAll[2] == "x") bit_owner_e = "checked";
              
  if(PermAll[3] == "r") bit_group_r = "checked";
  if(PermAll[4] == "w") bit_group_w = "checked";
  if(PermAll[5] == "x") bit_group_e = "checked";
             
  if(PermAll[6] == "r") bit_other_r = "checked";
  if(PermAll[7] == "w") bit_other_w = "checked";
  if(PermAll[8] == "x") bit_other_e = "checked";     


  $('#discrete_bit_8').prop('checked', ((PermAll[0] == "r") ?true:false));
  $('#discrete_bit_7').prop('checked', ((PermAll[1] == "w") ?true:false));
  $('#discrete_bit_6').prop('checked', ((PermAll[2] == "x") ?true:false));
  
  $('#discrete_bit_5').prop('checked', ((PermAll[3] == "r") ?true:false));
  $('#discrete_bit_4').prop('checked', ((PermAll[4] == "w") ?true:false));
  $('#discrete_bit_3').prop('checked', ((PermAll[5] == "x") ?true:false));
  
  $('#discrete_bit_2').prop('checked', ((PermAll[6] == "r") ?true:false));
  $('#discrete_bit_1').prop('checked', ((PermAll[7] == "w") ?true:false));
  $('#discrete_bit_0').prop('checked', ((PermAll[8] == "x") ?true:false));
   
  $('button[name=b_discrete_save]').click(function() {  
      take_discrete_chmod(mHost,mAction,octVal);
   });
}


function MandatHtmlBlock(mHost, mAction, mBlock){
    var MBlock = "";
    var LevelMand  = "Уровень_" + mBlock.data.mandat_rules.res_mandat_1;
    var LevelMandC = (mBlock.data.mandat_rules.res_mandat_2 == "0")?"Низкий":"Высокий"; 
    var CategoriesLevel  = "";
    var mandat_bits      = 0;
    var mandat_bit_labal = "";
    var MaskFlags = [0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00];
    var mandat_flags = ["ccnr", "ccnri","ehole", "whole"];  
    if(mBlock.data.mandat_rules.res_mandat_3 == "0x0")
      CategoriesLevel += "0x00 (не определены)";
    else{
      if(mBlock.data.mandat_rules.res_mandat_3.length){
        var mCatPos = 1;
        var FlagBytes  = parseInt(((mBlock.data.mandat_rules.res_mandat_3.length - 2)/2),10);
        if(((mBlock.data.mandat_rules.res_mandat_3.length - 2)%2))
          FlagBytes +=1;
        var FlagNibles = ((mBlock.data.mandat_rules.res_mandat_3.length - 2)%2);
        var Bcl;
        var NumByte = 0;
        var e = mBlock.data.mandat_rules.res_mandat_3.length;
        while(FlagBytes--){
           if(e ==  mBlock.data.mandat_rules.res_mandat_3.length && FlagNibles != 0){
             Bcl = "0x0" + mBlock.data.mandat_rules.res_mandat_3.substring(e - 1, e);
             e = e - 1;
           }
           else{
             Bcl = "0x" + mBlock.data.mandat_rules.res_mandat_3.substring(e - 2 ,e);
             e = e - 2;
           }
           var CLevel = hexToDec(Bcl); 
           //alert(Bcl);
           for(var w = 0; w < 8; w++){
             if(CLevel&(1<<w)){            
               CategoriesLevel += "Категория_" + mCatPos + "<br>";              
               MaskFlags[NumByte] |= (1<<w);
             }
             mCatPos++;   
           }
           NumByte++;
        }                 
      }
      if(mBlock.data.mandat_rules.res_mandat_3  == "0xffffffffffffffff")
        CategoriesLevel = "Все возможные (1-64)";
    }
    if(mBlock.data.mandat_rules.res_mandat_4 == "0x0")
      mandat_bit_labal += "0x00 (не определены)";    
    else{ 
      if(mBlock.data.mandat_rules.res_mandat_4.length){
        var CLev =  hexToDec(mBlock.data.mandat_rules.res_mandat_4);
        for(var e = 0; e < 4; e++){
          if((CLev&(1<<e))){
            mandat_bit_labal += mandat_flags[e] + "<br>";
            mandat_bits |= (1<<e);
          }         
        }
      }    
    }    
    LevelMandC += " (" + mBlock.data.mandat_rules.res_mandat_2 + ")";  
    
    
    $('#MandatObjects .tabs__content_discr').html("");          
    $('#MandatObjects .tabs__content_discr').load('html/mandat_tmp_devices.html', '.mandat_table_div', function(){                         
      var out = "";
      var iDivObj = 0;
      var iTimeObj = 0;		 
      var NamesData = mBlock.data;
      $('.device_tmp_cols_1').html(LevelMand);      
      $('.device_tmp_cols_2').html(LevelMandC);
      $('.device_tmp_cols_3').html(CategoriesLevel);
      $('.device_tmp_cols_4').html(mandat_bit_labal);
      MandatHtmlAddBlock(mHost, mAction, mBlock, MaskFlags);
    });  
}

function ACLHtmlBlock(mHost, mAction){
        
    $('.ResourceLocalObjectsSelectUser').click(function() {   
      ResourceLocalObjectsSelect(mHost,'Users','#audit_rule_who_id_',0,0);
   });
    $('.ResourceLocalObjectsSelectGroup').click(function() {             
      ResourceLocalObjectsSelect(mHost,'Groups','#audit_rule_who_id_',0,0);              
   });   
   $('button[name=b_acl_delete_all]').click(function() {             
      ResourceDeleteAcl(mHost,mAction,'',1);
   });
   $('button[name=b_acl_in]').click(function() { 
      $('#table_save_acl').dialog('close');
      ResourceAddAcl(mHost,mAction);      
      
   });
   $('button[name=b_acl_add]').click(function() {             
   }); 
}



function MandatHtmlAddBlock(mHost, mAction, MandatObjects, MaskFlags){
   var prop_mode = $('#filetype_').attr('value');
   if(prop_mode == "d"){
        $('#flag_ehole').attr('type', 'hidden');
        $('#flag_whole').attr('type', 'hidden');       
        //
        $('#flag_ccnr').attr('type', 'checkbox');
        $('#flag_ccnri').attr('type', 'checkbox');
        
        $('#l_ehole').text('');
        $('#l_whole').text('');

   }   
   if(prop_mode == "f"){
        $('#flag_ehole').attr('type', 'checkbox');
        $('#flag_whole').attr('type', 'checkbox');
        //
        $('#flag_ccnr').attr('type', 'hidden');
        $('#flag_ccnri').attr('type', 'hidden');
        
        $('#l_ccnr').text('');
        $('#l_ccnri').text('');
   }
    /**/
    for(i in MandatObjects.data.res_lev) {  
        var CheckLevel = (MandatObjects.data.mandat_rules.res_mandat_1 == i)?"selected":"";       
        $('#MandatObjectLevals').append("<option value=\'" + MandatObjects.data.res_lev[i].res_lev_id + "\' " + CheckLevel + ">" + MandatObjects.data.res_lev[i].res_lev_name + "</option>");
    }
   /**/
    var out_list1 = "";
    var mCateg = Number(MandatObjects.data.mandat_rules.res_mandat_3); 
    var FlagBytes  = parseInt(((MandatObjects.data.mandat_rules.res_mandat_3.length - 2)/2),10);
    var posBits = 0;
    var posBitsCat = 0;
    for(var i = 0; i < 8; i++) { 
      for(var e = 0; e < 8; e++){
        if(posBits in MandatObjects.data.res_cat){                        
            var CheckBit = (MaskFlags[i]&(1<<e))?"checked":"";
            var hexVal = (0x0000000000000000|(1<<posBits));
            if((posBits + 1)  in MandatObjects.data.res_cat){
              out_list1 += "<input name=\"all_categ_mandat_list\" type=\"checkbox\" id=\"flag_categ_" + posBits + "\" value=\"" + posBits + "\" " + CheckBit + "> " + posBits + "/0x" +  decToHex(hexVal) +  "<br>";  
            }
            
        }
        posBits++;
      }                 
    }    
    /**/
    for(var q = 0; q < 64; q++) {  
      var NonoliteLevel = (MandatObjects.data.mandat_rules.res_mandat_2 == q)?"selected":"";
      $('#MandatObjectMonolite').append("<option value=\'" + i + "\' " + NonoliteLevel + ">" + q + "</option>");
    }
    
    $('.out_list1').html(out_list1);
    var AttrFlags  = hexToDec(MandatObjects.data.mandat_rules.res_mandat_4);
    if(prop_mode == "d"){
      $('#flag_ccnr').prop('checked',  (AttrFlags&(1<<0))?true:false);
      $('#flag_ccnri').prop('checked', (AttrFlags&(1<<1))?true:false);
    }
    if(prop_mode == "f"){
      $('#flag_ehole').prop('checked', (AttrFlags&(1<<2))?true:false);
      $('#flag_whole').prop('checked', (AttrFlags&(1<<3))?true:false);
    }
    
    $('button[name=b_mandat_in]').click(function() {    
       $('#table_save_mandat').dialog('close');
      ResourceMandatObjectsSave(mHost,mAction);
   });
    
}


function take_discrete_chmod(mHost, mAction, mOld){
 var s_bit_new   = 0;
 var s_bit_suid    =  (document.getElementById("s_bit_suid").value == "checked")?true:false;
 var s_bit_sguid   =  (document.getElementById("s_bit_sguid").value == "checked")?true:false;
 var s_bit_sticky  =  (document.getElementById("s_bit_sticky").value == "checked")?true:false;
 
 var s_bit_suid_new    =  (document.getElementById("s_bit_suid").checked)?true:false;
 var s_bit_sguid_new   =  (document.getElementById("s_bit_sguid").checked)?true:false;
 var s_bit_sticky_new  =  (document.getElementById("s_bit_sticky").checked)?true:false;
 
 var chown_own_new   = $("#discrete_who_id_own").find("option:selected").attr('value'); 
 var chown_group_new = $("#discrete_who_id_group").find("option:selected").attr('value');
 
 var chown_own_old   = $("#discrete_who_id_own").attr('save'); 
 var chown_group_old = $("#discrete_who_id_group").attr('save');
 

 if((s_bit_suid   != s_bit_suid_new)){ 
   s_bit_new |=(1<<0);     
 }
 if((s_bit_sguid  != s_bit_sguid_new)){
   s_bit_new |=(1<<1);      
 }
 if((s_bit_sticky != s_bit_sticky_new)){
    s_bit_new |=(1<<2);       
 }
    
  var nNewDiscreteVal = 0;
  for(var k = 0; k < 9; k++){      
    if(document.getElementById("discrete_bit_" + k).checked)
      nNewDiscreteVal|=(1<<k);
  }
  var octValNew = (nNewDiscreteVal).toString(8); 
  if((octValNew != mOld) ||
     (s_bit_new != 0)    ||
     (chown_own_new != chown_own_old) ||
     (chown_group_new != chown_group_old)
     
){
      /*update Discrete*/
    var rec = "";
  if ($('#s_recursive').is(':checked')) 
    rec = "-R";   
    //alert(octValNew);
    ResourceDiscrete(mHost, mAction, mOld, octValNew, rec, s_bit_new, chown_own_new, chown_group_new, chown_own_old, chown_group_old);
  }    
}

function Check(element) {
  element.setAttribute('src', 'images/loader.gif');
}

function CheckLoader(element) {
  $(element).children('.system_image').prop('src', 'images/loader.gif');
}

function CheckLoaderThis(element) {
  $(element).prop('id', 'file_go');
}

function showInfoDev(element){
    document.getElementById(element).style.display = (document.getElementById(element).style.display == "block")?"none":"block";
}


function decToHex(n){ return Number(n).toString(16); }

function hexToDec (hex){ return parseInt(hex,16); }

function UpdateRulesSaveEditForm(){
    $('img[name=append_audit_dir]').click(function() {  
      $('#table_save_audit').dialog('close');
      var idRule = $(this).attr('id'); 
      var host =  $('.resource_host').text();
      var oath_by_host  = $('.resource_path').text();
      
      SaveResourceDiskModeAction(host, idRule, 'i', '');            
   });    
    
}

function UpdateRulesEditForm (){
  $(function() {       
    $('.audit_rule_subject_').change(function() {      
      var text = $(this).find("option:selected").attr('value'); 
      var mTypeSub = "";
      var mDomain = 0;
      if(text == "o"){
        $('#audit_rule_who_id_').html("");
        $('#audit_rule_who_id_').append('<option value="o">-</option>');
        return;
      }
      else if(text == "au"){
        mTypeSub = "Users";
        mDomain = 1;
      }
      else if(text == "ag"){
        mTypeSub = "Groups";          
        mDomain = 1;
      }
      else if(text == "lu"){
        mTypeSub = "Users";
        mDomain = 0;
      }
      else if(text == "lg"){
        mTypeSub = "Groups";          
        mDomain = 0;
      }
      var host =  $('.resource_host').text();
      ResourceLocalObjectsSelect(host, mTypeSub, '#audit_rule_who_id_', mDomain, 1);
   });
    
    $('#add_audit_rule_submit_id_').click(function() {  
      $('#table_save_audit').dialog('close');
      var allRules ="";
      var host =  $('.resource_host').text();
      var oath_by_host  = $('.resource_path').text();
      var subject_audit = $('.audit_rule_subject_').find("option:selected").attr('value');
      var subject_name  = $('#audit_rule_who_id_').find("option:selected").attr('value');
      
      var checked_sucess = "";
      var checked_fail   = "";
      $('#audit_success_checkbox_list_ input:checkbox:checked').each(function() {
        checked_sucess += $(this).val();        
      });
      $('#audit_fail_checkbox_list_ input:checkbox:checked').each(function() {
        checked_fail += $(this).val();
      });
      if(subject_audit != "o")
        allRules += subject_audit.substring(1,2);
      else
        allRules += subject_audit;
      if(subject_audit != "o")
        allRules += ":\'" + subject_name + "\'"; 
      if(checked_sucess != "")
        allRules += ":" + checked_sucess;
      else
        allRules += ":0";
      if(checked_fail != "")
        allRules += ":" + checked_fail; 
      else
        allRules += ":0";  
      var RecursiveFlag = $('#s_recursive').is(':checked');
      AuditActionAdd(host, oath_by_host, allRules, RecursiveFlag, " "); 
      //showInfoDev('table_save_audit');
            
   });
    
    $('#delete_audit_rule_id_').click(function() {      
       showInfoDev('table_save_audit');
   });
    
    $('#audit_rule_subject_clear').click(function() {  
      var host =  $('.resource_host').text();
      var oath_by_host  = $('.resource_path').text();
      var RecursiveFlag = $('#s_recursive').is(':checked');
      AuditActionAdd(host, oath_by_host, "-", RecursiveFlag, " "); 
   });
    
    $('img[name=rem_audit]').click(function() {             
      var id = $(this).attr('id');       
      var host =  $('.resource_host').text();
      var oath_by_host  = $('.resource_path').text();
      var RecursiveFlag = $('#s_recursive').is(':checked');
      AuditActionAdd(host, oath_by_host, id, RecursiveFlag, " "); 
   });
    
     $('img[name=append_audit_dir]').click(function() {             
      var id = $(this).attr('id');       
      var host =  $('.resource_host').text();
      //showInfoDev("table_save_audit_folder");
      $('.rules_hidden_dir').dialog('close');
      AuditAction(host, id,' ');
   });
    
    
    $('#s_recursive').click(function(){ 
      if ($('#s_recursive').is(':checked')){
        g_checked_rec = true;
      } else {
        g_checked_rec = false;
      }
   });
    
 });
}

function LoadTemplateContentHosts(ListOfNames){
  var iTimeObj = 0;	
   /*
   $('.device_tmp_cols_6').html(function(index, oldText){                                                 
     oldText = "<span><font color=\"grey\">Идет запрос статуса</font></span>";
     return oldText;
  });
  */

  $('.device_tmp_cols_1').text(function(index, oldText){                                 
    oldText = (index + 1).toString();
    return oldText;
  });
  $('.device_tmp_cols_2').text(function(index, oldText){                             
    oldText    =  ListOfNames.data[index].host_hame;
    $(this).prop('id',ListOfNames.data[index].host_hame);
    return oldText;
  });
  $('.device_tmp_cols_3').text(function(index, oldText){                             
    oldText    =  ListOfNames.data[index].host_ip;               
    return oldText;
  });
  $('.device_tmp_cols_4').html(function(index, oldText){     
    oldText = ""; 
    for(j in ListOfNames.data[index].details){
      if(j == 1 || j == 6 || j == 7){         
        var TimeObj = ListOfNames.data[index].details[j].split(': '); 
        if(TimeObj[0] == "Изменено"){
          var tmp  = TimeObj[1].split(', ');
          tmp  = tmp[1].split(' +');
          var tmpCheck = formatDate(tmp[0].toString());
          if(tmpCheck.match('NaN')) tmpCheck = tmp[0];
                               
          oldText += "<span>Изменено: " + tmpCheck + "</span><br>"
        } 
        else{
          if( ListOfNames.data[index].details[j] == "Роль: Первичный контроллер домена")
            oldText += "<span class=\"domain_span\">Первичный контроллер домена</span><br>"
          else  
            oldText += "<span>" + ListOfNames.data[index].details[j]   + "</span><br>"
        }
      }
    }
    return oldText;
  });                        
  $('.device_tmp_cols_5').text(function(index, oldText){                             
    for(j in ListOfNames.data[index].details){
      var TimeObj = ListOfNames.data[index].details[j].split(': '); 
      if(TimeObj[0] == "Создано"){
        var tmp  = TimeObj[1].split(', ');
        tmp  = tmp[1].split(' +');
        var tmpCheck = formatDate(tmp[0].toString());
        if(tmpCheck.match('NaN')) tmpCheck = tmp[0];
        iTimeObj = tmpCheck;
      }            
    }                 
    oldText    =   iTimeObj;
    return oldText;
  });
  $('.device_tmp_cols_7').html(function(index, oldText){
      $(this).attr('id', ListOfNames.data[index].host_hame);
    //  oldText    = "<span  id=\"glowtext\"><img class=\"system_image\" src=\"images/delete.png\"></span>";
    /*oldText    = "<span  id=\"glowtext\" onclick=\"hostAction('" + ListOfNames.data[index].host_hame + "','/');\"><img class=\"system_image\" src=\"images/settings-icon.png\" onclick=\"Check(this);\"></span>";*/
  return oldText;
  });;
  $('.device_tmp_cols_8').html(function(index, oldText){ 
    $(this).attr('id', ListOfNames.data[index].host_hame);
    //oldText    = "<span  id=\"glowtext\"><img class=\"system_image\" src=\"images/delete.png\"></span>";      
    //oldText    = "<span id=\"glowtext\" onclick=\"AuditAction('" + ListOfNames.data[index].host_hame + "','/','');\"><img class=\"system_image\" //src=\"images/settings-icon.png\" onclick=\"Check(this);\"></span>";
    return oldText;
  });      
  $('.device_tmp_cols_9').html(function(index, oldText){ 
    $(this).attr('id', ListOfNames.data[index].host_hame);
    //oldText    = "<span  id=\"glowtext\"><img class=\"system_image\" src=\"images/delete.png\"></span>";
    //oldText    = "<span id=\"glowtext\" onclick=\"CDROMActionList('" + ListOfNames.data[index].host_hame + "','cdrom');\"><img class=\"system_image\" src=\"images/settings-icon.png\" onclick=\"Check(this);\"></span>";
    return oldText;
  });   
  $('.device_tmp_cols_10').html(function(index, oldText){ 
    $(this).attr('id', ListOfNames.data[index].host_hame);
    //oldText    = "<span  id=\"glowtext\"><img class=\"system_image\" src=\"images/delete.png\"></span>";
    //oldText    = "<span id=\"glowtext\" onclick=\"SaveResourceAction('" + ListOfNames.data[index].host_hame + "');\"><img class=\"system_image\" src=\"images/settings-icon.png\" onclick=\"Check(this);\"></span>";
    return oldText;
  }); 
}

function LoadTemplateContentDir(mHost, url, ListOfNames, mFrameType){
  $(".left_file_box_icon").html(function(index, oldText){ 
    if(index > 0){
      oldText = "<img class=\"system_image\" name=\"append_audit_dir\" id=\"" + ListOfNames.data[index - 1].res_base + "\" src=\"images/append.png\">";                                
   }
    return oldText;
  });
  $(".left_file_box").html(function(index, oldText){ 
    if(index > 0){
      var pathObj;
      var pathFull;
      if(ListOfNames.data[index - 1].res_type == "d"){       
        pathObj = ListOfNames.data[index - 1].res_hame;              
        if(ListOfNames.data[index - 1].res_hame != ".."){
          oldText = "<span id=\"glowtext\" onclick=\"DirAction(\'" + mHost + "\',\'" + ListOfNames.data[index - 1].res_base + "/\',\'" + mFrameType + "\');\"><img class=\"system_image\" id=\"folder_img_gr\" src=\"images/Folder-icon.png\"> " + pathObj + "</span>";
        }
      }
      else{                     
        pathObj = ListOfNames.data[index - 1].res_hame;
        oldText = '<span id=\"glowtext\" onclick=\"take_path(\'' + mHost + '\' , \''+ ListOfNames.data[index - 1].res_base + '\');\"><img class=\"system_image\" id=\"file_img_gr\" src=\"images/state_write.png\">' + pathObj + '</span>';
      }
    }
    else
      oldText = "<span id=\"glowtext\"  onclick=\"DirAction(\'" + mHost + "\',\'"+ url + "/\',\'" + mFrameType + "\');\"><img class=\"system_image\" id=\"folder_img_gr\" src=\"images/Folder-icon.png\"></span>";
      return oldText;
    });     
}



function LoadTemplateFilesDevuce(mHost, url, ListOfNames){
    
  $(".left_file_box_icon").html(function(index, oldText){ 
    if(index > 0){
      oldText = "<span id=\"glowtext\" onclick=\"ResourceAttribute(\'" + mHost + "\',\'" + ListOfNames.data[index - 1].res_base + "\',1, \'" + ListOfNames.data[index - 1].res_type + "\');\"><img class=\"system_image\" src=\"images/settings-icon.png\"></span>";
      return oldText;                               
   }
    return oldText;
  });

  $(".left_file_box").html(function(index, oldText){ 
    if(index > 0){
      var pathObj;
      var pathFull;
      if(ListOfNames.data[index - 1].res_type == "d"){       
        pathObj = ListOfNames.data[index - 1].res_hame;              
        if(ListOfNames.data[index - 1].res_hame != ".."){
          oldText = "<span id=\"glowtext\" onclick=\"hostAction(\'" + mHost + "\',\'" +
                ListOfNames.data[index - 1].res_base + "/\');\"><img class=\"system_image\" id=\"folder_img_gr\" src=\"images/Folder-icon.png\"> " + pathObj + "</span>";
        }
      }
      else{                     
        pathObj = ListOfNames.data[index - 1].res_hame;
        oldText = '<span id=\"glowtext\" onclick=\"take_path(\'' + mHost + '\' , \''+ ListOfNames.data[index - 1].res_base + '\');\"><img class=\"system_image\" id=\"file_img_gr\" src=\"images/state_write.png\">' + pathObj + '</span>';
      }
    }
    else
      oldText = "<span id=\"glowtext\"  onclick=\"hostAction(\'" + mHost + "\',\'"+ url + "/\');\"><img  class=\"system_image\" id=\"folder_img_gr\" src=\"images/Folder-icon.png\"></span>";
    
      return oldText;
  }); 
  /*
  $('.system_image').click(function() {  
      $(this).prop('src', 'images/loader.gif');
    });  
 */
}



function USBActionList(mHost, mDevice, mRule) {
  //alert('Resources!');  
  $("#tableHostPrint").html("<img class=\"system_image\" id=\"file_go\" src=\"images/loader.gif\"><h3>Обновление данных конфигурации...</h3>");
  var xmlhttp = getXmlHttp(); // Создаём объект XMLHTTP
  xmlhttp.timeout = 60000;
  xmlhttp.ontimeout = function() {
    xmlhttp.abort();
    alert("Данные не получены от сервера (Timeout request).");
  }
  xmlhttp.open('POST', '/action', true); // Открываем асинхронное соединение
  xmlhttp.setRequestHeader('Cache-Control','max-age=3600');
  xmlhttp.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded');  
  var Resp = "resourcebyhost=devbusdriver";
  if(mHost.length)
    Resp += "&resourcehost=" + mHost; 
  if(mDevice.length)
    Resp += "&dev=" + mDevice; 
  if(mRule.length)
    Resp += "&rule=" + mRule; 

  xmlhttp.send(Resp);   
  xmlhttp.onreadystatechange = function() { // Ждём ответа от сервера
    if (xmlhttp.readyState == 4) { // Ответ пришёл        
      if(xmlhttp.status == 200) { // Сервер вернул код 200 (что хорошо)          
          //alert(xmlhttp.responseText);                    
;         var out = "";
          var iDivObj = 0;
		  var ListOfNames = JSON.parse(xmlhttp.responseText);
          //alert(xmlhttp.responseText);
          if(mRule != 'list'){                              
            if(ListOfNames.data.update == "OK"){
              USBActionList(mHost, 'usb','list');
            }                        
            else
              alert('Не удалось обновить данные конфигурации.');          
          }
          else{        
              $('#tableHostPrint').html("");                      
              $('#tableHostPrint').load('html/usb_tmp_devices.html', '.group_list_all_div', function(){              	 
		      var NamesData   = ListOfNames.data;                    
            
              var file_row_tmpl  = $(".tb_group_dev_list_tmpl").find( $("tbody > tr.device_row_next")).html();             
              $('.header1').html("<span>Параметры конфигурации: <p class=\"resource_host\">" + mHost + "</p></span>");
              $('.header2').html("<span id=\"glowtext\" onclick=\"CheckLoader(this);hostlistAld();\"><img class=\"system_image\" id=\"back_go\"  onclick=\"Check(this);\"  src=\"images/back.png\">Вернуться к списку устройств</span>");                
              LoadTemplateDataDevUSB(mHost, ListOfNames);            
            });           
         }

      }
      else if (xmlhttp.status == 401) { // Нет авторизации
        location.href = '/';
      }
      else {
        //обработка ошибки
        if(this.statusText.length)
          alert('error: ' + this.status ? this.statusText : 'query failed');
        return;
      }
    }
  };
}





function USBRulesActionList(mHost, mDevice, mRule, mType, mUser, mGroup, mDiscrete, mExst) {
  //alert(mExst);  
  $("#tableHostPrint").html("<img class=\"system_image\" id=\"file_go\" src=\"images/loader.gif\"><h3>Обновление данных конфигурации...</h3>");
  var xmlhttp = getXmlHttp(); // Создаём объект XMLHTTP
  xmlhttp.timeout = 60000;
  xmlhttp.ontimeout = function() {
    xmlhttp.abort();
    alert("Данные не получены от сервера (Timeout request).");
  }
  xmlhttp.open('POST', '/action', true); // Открываем асинхронное соединение
  xmlhttp.setRequestHeader('Cache-Control','max-age=3600');
  xmlhttp.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded');  
  var Resp = "";
  if(mType == 0)
    Resp += "resourcebyhost=dev_disk_serial";
  else if(mType == 1){
    Resp += "resourcebyhost=mod_usb_rules_serial";
    Resp += "&typerule=del";
  }
  else if(mType == 2){
    Resp += "resourcebyhost=mod_usb_rules_serial";
    Resp += "&typerule=add";
  }  
  if(mHost.length)
    Resp += "&resourcehost=" + mHost; 
  if(mDevice.length)
    Resp += "&dev=" + mDevice; 
  if(mRule.length && mType != 0)
    Resp += "&rule=" + mRule; 
  if(mUser.length && mType != 0)
    Resp += "&user=" + mUser; 
  if(mGroup.length && mType != 0)
    Resp += "&group=" + mGroup; 
  if(mDiscrete.length && mType != 0)
    Resp += "&discrete=" + mDiscrete; 
 
 

  xmlhttp.send(Resp);   
  xmlhttp.onreadystatechange = function() { // Ждём ответа от сервера
    if (xmlhttp.readyState == 4) { // Ответ пришёл        
      if(xmlhttp.status == 200) { // Сервер вернул код 200 (что хорошо)          
          //alert(xmlhttp.responseText);                    
;         var out = "";
          var iDivObj = 0;
		  var ListOfNames = JSON.parse(xmlhttp.responseText);
          //alert(xmlhttp.responseText);
          if(mType != 0){                              
            if(ListOfNames.data.update == "OK"){               
              //USBRulesActionList(mHost, mDevice, mRule, 0, '', '', 0, 0);
              
              USBRulesActionListNeedTouch(mHost, '','', 0);
            }                        
            else
              alert('Не удалось обновить данные конфигурации.');          
          }
          else{        
              $('#tableHostPrint').html("");                      
              $('#tableHostPrint').load('html/usb_tmp_devices_rules.html', '.group_list_all_div', function(){              	 
		      var NamesData   = ListOfNames.data;                    
            
              var file_row_tmpl  = $(".tb_group_dev_list_tmpl").find( $("tbody > tr.device_row_next")).html(); 
              $(".tb_group_dev_list_tmpl").find( $("tbody > tr.device_row_next")).remove();
            
              $('.header1').html("<span>Параметры конфигурации: <p class=\"resource_host\">" + mHost + "</p></span>");
              $('.header2').html("<span id=\"glowtext\" onclick=\"CheckLoader(this);hostlistAld();\"><img class=\"system_image\" id=\"back_go\"  onclick=\"Check(this);\"  src=\"images/back.png\">Вернуться к списку устройств</span>");      
              /*
              if(mExst == 1){
                 $('.if_rules_storage').html("<span>Файл правил UDEV /etc/udev/rules.d/99-usb.rules  <button  class=\"dev_defbutton\"  name=\"del_usb_file_rule\">Удалить файл</button></span>"); 
              }*/            
              for(i in ListOfNames.data) {
                $('.tb_group_dev_list_tmpl tr:last').after("<tr>" + file_row_tmpl + "</tr>");
                iDivObj += 1;              
              }
              $('#udevd_rule_storage').html(function(index, oldText){              
                if(mExst == 1)
                  $(this).prop('checked',true);
                 else
                  $(this).prop('checked',false);
                  return oldText;
              })        
              $('#udevd_rule_storage').change(function(){  
               var id = $(this).attr('id');              
               if($(this).is(':checked')){;
                  USBRulesActionListTouch(mHost, '', '', 0);                    
               }
                else{
                  var resIs = "Вы хотите удалить на хосте локальный файл правил UDEV?";
                  var isYes = confirm(resIs);
                  if(isYes){
                    UdevLocalClear(mHost);    
                  }                       
                }
              });

              
              ResourceLocalObjectsSelect(mHost,'Groups','#audit_rule_group_id_',0,0);
              ResourceLocalObjectsSelect(mHost,'Users','#audit_rule_who_id_',0,0);
              
              /**/
              LoadTemplateDataDevUSBRules(mHost, ListOfNames);  
            });           
         }

      }
      else if (xmlhttp.status == 401) { // Нет авторизации
        location.href = '/';
      }
      else {
        //обработка ошибки
        if(this.statusText.length)
          alert('error: ' + this.status ? this.statusText : 'query failed');
        return;
      }
    }
  };
}



function USBRulesActionListTouch(mHost, mDevice, mRule, mType) {
  //alert('Resources!');  
  $("#tableHostPrint").html("<img class=\"system_image\" id=\"file_go\" src=\"images/loader.gif\"><h3>Обновление данных конфигурации...</h3>");
  var xmlhttp = getXmlHttp(); // Создаём объект XMLHTTP
  xmlhttp.timeout = 60000;
  xmlhttp.ontimeout = function() {
    xmlhttp.abort();
    alert("Данные не получены от сервера (Timeout request).");
  }
  xmlhttp.open('POST', '/action', true); // Открываем асинхронное соединение
  xmlhttp.setRequestHeader('Cache-Control','max-age=3600');
  xmlhttp.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded');  
  var Resp = "resourcebyhost=dev_usb_rules_touch";
  if(mHost.length)
    Resp += "&resourcehost=" + mHost; 
 
  xmlhttp.send(Resp);   
  xmlhttp.onreadystatechange = function() { // Ждём ответа от сервера
    if (xmlhttp.readyState == 4) { // Ответ пришёл        
      if(xmlhttp.status == 200) { // Сервер вернул код 200 (что хорошо)          
          //alert(xmlhttp.responseText);                    
;         var out = "";
          var iDivObj = 0;
		  var ListOfNames = JSON.parse(xmlhttp.responseText);
            //alert(xmlhttp.responseText);                           
            if(ListOfNames.data.update == "OK" || ListOfNames.data.update == "FILE_EXIST"){            
                USBRulesActionList(mHost, mDevice, mRule, 0, '', '', 0, 1);                             
            }                        
            else{
              alert('Не удалось найти файл правил udev.');          
              USBRulesActionList(mHost, mDevice, mRule, 0, '', '', 0, 0)
            }
        
      }
      else if (xmlhttp.status == 401) { // Нет авторизации
        location.href = '/';
      }
      else {
        //обработка ошибки
        if(this.statusText.length)
          alert('error: ' + this.status ? this.statusText : 'query failed');
        return;
      }
    }
  };
}



function USBRulesActionListNeedTouch(mHost, mDevice, mRule, mType) {
  //alert('Resources!');  
  //$("#tableHostPrint").html("<img class=\"system_image\" id=\"file_go\" src=\"images/loader.gif\"><h3>Обновление данных конфигурации...</h3>");
  var xmlhttp = getXmlHttp(); // Создаём объект XMLHTTP
  xmlhttp.timeout = 60000;
  xmlhttp.ontimeout = function() {
    xmlhttp.abort();
    alert("Данные не получены от сервера (Timeout request).");
  }
  xmlhttp.open('POST', '/action', true); // Открываем асинхронное соединение
  xmlhttp.setRequestHeader('Cache-Control','max-age=3600');
  xmlhttp.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded');  
  var Resp = "resourcebyhost=dev_need_touch";
  if(mHost.length)
    Resp += "&resourcehost=" + mHost; 
 
  xmlhttp.send(Resp);   
  xmlhttp.onreadystatechange = function() { // Ждём ответа от сервера
    if (xmlhttp.readyState == 4) { // Ответ пришёл        
      if(xmlhttp.status == 200) { // Сервер вернул код 200 (что хорошо)          
          //alert(xmlhttp.responseText);                    
;         var out = "";
          var iDivObj = 0;
		  var ListOfNames = JSON.parse(xmlhttp.responseText);
            //alert(xmlhttp.responseText);         
            if(ListOfNames.data.update == "FILE_EXIST"){  
              USBRulesActionList(mHost, mDevice, mRule, 0, '', '', 0, 1);                            
            }                        
            else if(ListOfNames.data.update == "FILE_NO_EXIST"){  
              /*
              var resIs = "Не обнаружен локально настроенный файл правил USB-устройств UDEV.\r\nФайл правил будет блокировать все неавторизованные устройства (по умолчанию).\r\nВы хотите создать на хосте локальный файл правил UDEV?";
              var isYes = confirm(resIs);
              if(isYes){
                USBRulesActionListTouch(mHost, mDevice, mRule, mType);                 
              }                           
              else
                USBRulesActionList(mHost, mDevice, mRule, 0, '', '', 0, 0); 
              */
              USBRulesActionList(mHost, mDevice, mRule, 0, '', '', 0, 0); 
            }                   
      }
      else if (xmlhttp.status == 401) { // Нет авторизации
        location.href = '/';
      }
      else {
        //обработка ошибки
        if(this.statusText.length)
          alert('error: ' + this.status ? this.statusText : 'query failed');
        return;
      }
    }
  };
}



function hostlistAldPolicy() {
  //alert('Resources!');  
  //$("#tableHostPrintALD").html("<img class=\"system_image\" id=\"file_go\" src=\"images/loader.gif\"><h3>Обновление данных...</h3>");
  var xmlhttp = getXmlHttp(); // Создаём объект XMLHTTP
  xmlhttp.timeout = 60000;
  xmlhttp.ontimeout = function() {
    xmlhttp.abort();
    alert("Данные не получены от сервера (Timeout request).");
  }
  xmlhttp.open('POST', '/policy', true); // Открываем асинхронное соединение
  xmlhttp.setRequestHeader('Cache-Control','max-age=3600');
  xmlhttp.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded');  
  var Resp = "domainpolicy=domaindevlist";
  xmlhttp.send(Resp);   
  xmlhttp.onreadystatechange = function() { // Ждём ответа от сервера
    if (xmlhttp.readyState == 4) { // Ответ пришёл        
      if(xmlhttp.status == 200) { // Сервер вернул код 200 (что хорошо)          
          //alert(xmlhttp.responseText);                    
;         var out = "";
          var iDivObj = 0;
		  var ListOfNames = JSON.parse(xmlhttp.responseText);
          //alert(xmlhttp.responseText);
          
          $('#tableHostPrintALD').html("");                      
          $('#tableHostPrintALD').load('html/ald_tmp_devices.html', '.host_list_all_div', function(){              	 
		    var NamesData   = ListOfNames.data;                                
            var file_row_tmpl  = $(".tb_ald_list_tmpl").find( $("tbody > tr.ald_device_row_next")).html(); 
            $(".tb_ald_list_tmpl").find( $("tbody > tr.ald_device_row_next")).remove();
    
            for(i in ListOfNames.data) {
                $('.tb_ald_list_tmpl tr:last').after("<tr>" + file_row_tmpl + "</tr>");
                iDivObj += 1;
              
            }                 
            LoadTemplateDataPolicy(ListOfNames);            
             $('.div_writer_form').load('html/append_ald_tmp_devices.html', '#ald_users_usb_device', function(){                    
                  ResourceLocalObjectsSelect('127.0.0.1','Users','#ald_audit_rule_who_id_',1,1);
                  ResourceLocalObjectsSelect('127.0.0.1','Groups','#ald_audit_rule_group_id_',1,1);
                    $('button[name=ald_save_user_usb_group]').attr('id',1);
                    $('button[name=ald_save_user_usb_group]').click(function() { 
                         if(AppendhostlistAldPolicy() == 0)      
                           $('#ald_users_usb_device').dialog('close');  
                    });
                    
                    $('.div_rule_writer_form').load('html/append_ald_rule_tmp_devices.html', '#rule_host_list_all_div', function(){                           
                      $('.div_rule_find_form').load('html/find_ald_rule_tmp_devices.html', '#find_rule_host_list_all_div', function(){                         
                              $('#find_rule_host_list_all_div','.ui-dialog').remove();    
	                          $('button[name=find_rule_add_ald_usb_group]').click(function(e) {
                                                            
                              $('#find_rule_host_list_all_div').dialog({ 
                                width : 600,
                                height: 300   
                            });    
                              });
                              RuleDomainResourceLocalObjectsSelect('#find_ald_audit_rule_group_id_');
                              $('#find_ald_audit_rule_group_id_').change(function() {   
                                  var id_dev = $('#find_ald_audit_rule_group_id_').find("option:selected").attr('value');
                                  DomainFindDevRuleActionList(id_dev);
                              });                                                                                              
                              $('#find_rule_host_list_all_div').dialog('open');

                      });                                                  
                          $('button[name=rule_ald_save_user_usb_group]').click(function() { 
                              var tmpDevRuleId = $('#_ald_usb_id').val();
                              var fullTmpRule = '';
                              var prop_mode = $('button[name=ald_save_user_usb_group]').attr('id');
                              if(prop_mode !=2 ){ 
                                if(tmpDevRuleId != '')
                                  fullTmpRule = "ENV{ID_SERIAL_SHORT}==\"" + tmpDevRuleId + "\"";
                                if(tmpDevRuleId != ''){
                                  $('.rule_tb_ald_list_tmpl tr:last').after("<tr id=\"tr_" + tmpDevRuleId + "\" class=\"rule_ald_device_row_next\"><td class=\"rule_ald_device_tmp_cols_2\">" + tmpDevRuleId + "</td><td class=\"rule_ald_device_tmp_cols_3\">" + fullTmpRule + "</td><td class=\"rule_ald_device_tmp_cols_4\"><span id=\"glowtext\"><img name=\"ruledeldevedomain\" class=\"system_image\" id=\"" + tmpDevRuleId + "\" src=\"images/delete_icon.png\"></span></td></tr>").ready(function () {      
                                      
                                     $('img[name=ruledeldevedomain]').click(function() {                               
                                       var tmpDevRuleId = $('#_ald_usb_id').val();                              
                                       var prop_mode = $('button[name=ald_save_user_usb_group]').attr('id');
                                         if(prop_mode !=2 ){
                                           var d_mode = $(this).attr('id');
                                           $(document.getElementById("tr_" + d_mode)).remove();                                
                                        }
                                      });
                                  });        
                                  $('#_ald_usb_id').val('');
                                }
                                else 
                                  alert('Не заполнены данные идентификатора устройства!');
                              }
                              else{                                  
                               if(tmpDevRuleId == ""){
                                  alert('Для устройства не задано правило!');
                                  return;
                              }
                              var cnt_group = 0;
                             $(".rule_ald_device_tmp_cols_2").html(function(index, oldText){ 
                               if(oldText == tmpDevRuleId){
                                 alert('Правило с таким идентификатором уже добавлено!');
                                 cnt_group = 1;
                               }               
                               return oldText;
                            });
                            if(cnt_group ==1) return;
                              if(prop_mode == 2 ){ 
                                 dev_name_id = $('#_ald_usb_name').val();
                                DomaiinTemplateRuleAdd(dev_name_id, tmpDevRuleId);          
                              }    
                                                                
                            }
                              
                        });


                    }); 

             }); 
             
             
          });
           
         
          

      }
      else if (xmlhttp.status == 401) { // Нет авторизации
        location.href = '/';
      }
      else {
        //обработка ошибки
        if(this.statusText.length)
          alert('error: ' + this.status ? this.statusText : 'query failed');
        return;
      }
    }
  };
}


function DelhostlistAldPolicy(PolicyName) {
  //alert('Resources!');  
  $("#tableHostPrintALD").html("<img class=\"system_image\" id=\"file_go\" src=\"images/loader.gif\"><h3>Обновление данных...</h3>");
  var xmlhttp = getXmlHttp(); // Создаём объект XMLHTTP
  xmlhttp.timeout = 60000;
  xmlhttp.ontimeout = function() {
    xmlhttp.abort();
    alert("Данные не получены от сервера (Timeout request).");
  }
  xmlhttp.open('POST', '/policy', true); // Открываем асинхронное соединение
  xmlhttp.setRequestHeader('Cache-Control','max-age=3600');
  xmlhttp.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded');  
  var Resp = "domainpolicy=rmdev";
  if(PolicyName.length)
    Resp += "&devname=" + PolicyName; 
  xmlhttp.send(Resp);         
  xmlhttp.onreadystatechange = function() { // Ждём ответа от сервера
    if (xmlhttp.readyState == 4) { // Ответ пришёл        
      if(xmlhttp.status == 200) { // Сервер вернул код 200 (что хорошо)          
          //alert(xmlhttp.responseText);                    
;         var out = "";
          var iDivObj = 0;
		  var ListOfNames = JSON.parse(xmlhttp.responseText);
          //alert(xmlhttp.responseText);
          if(ListOfNames.data.update != "OK"){
            var err_p = "Ошибка удаления устройства из политики домена ALD:\r\n";
            err_p +=  ListOfNames.data.update;
            alert(err_p);    
          }
          hostlistAldPolicy();                    

      }
      else if (xmlhttp.status == 401) { // Нет авторизации
        location.href = '/';
      }
      else {
        //обработка ошибки
        if(this.statusText.length)
          alert('error: ' + this.status ? this.statusText : 'query failed');
        return;
      }
    }
  };
}



function DomaiinTemplateRuleAdd(dev_name_id, id){
  //  $("#tableHostPrintALD").html("<img class=\"system_image\" id=\"file_go\" src=\"images/loader.gif\"><h3>Обновление данных...</h3>");
  var xmlhttp = getXmlHttp(); // Создаём объект XMLHTTP
  xmlhttp.timeout = 60000;
  xmlhttp.ontimeout = function() {
    xmlhttp.abort();
    alert("Данные не получены от сервера (Timeout request).");
  }
  xmlhttp.open('POST', '/policy', true); // Открываем асинхронное соединение
  xmlhttp.setRequestHeader('Cache-Control','max-age=3600');
  xmlhttp.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded');  
  var Resp = "domainpolicy=addproperty";
  if(dev_name_id.length)
    Resp += "&devname=" + dev_name_id; 
  if(id.length)
    Resp += "&devproperty=" + id;
  xmlhttp.send(Resp); 
  xmlhttp.onreadystatechange = function() { // Ждём ответа от сервера
    if (xmlhttp.readyState == 4) { // Ответ пришёл        
      if(xmlhttp.status == 200) { // Сервер вернул код 200 (что хорошо)          
          //alert(xmlhttp.responseText);                    
;         var out = "";
          var iDivObj = 0;
		  var ListOfNames = JSON.parse(xmlhttp.responseText);
          if(ListOfNames.data.update != "OK"){
            var err_p = "Ошибка добавления правила из политики домена ALD:\r\n";
            err_p +=  ListOfNames.data.update;
            alert(err_p);    
            return 1;
          }
          $(".rule_ald_device_tmp_cols_2").parent().html(function(index, oldText){     
             $(this).remove();
            return oldText;
         });
          $('.rule_tb_ald_list_tmpl tr:last').after("<tr class=\"rule_ald_device_row_next\"><td class=\"rule_ald_device_tmp_cols_2\" id=\"\"></td>\<td class=\"rule_ald_device_tmp_cols_3\" id=\"\"></td><td class=\"rule_ald_device_tmp_cols_4\" id=\"\"></td></tr>").ready(function () {      
            ViewhostlistAldPolicy(dev_name_id);                        
         });    
          return 0;
      }
      else if (xmlhttp.status == 401) { // Нет авторизации
        location.href = '/';
      }
      else {
        //обработка ошибки
        if(this.statusText.length)
          alert('error: ' + this.status ? this.statusText : 'query failed');
        return 2;
      }
    }
  };
    
    
}

function DomaiinTemplateRuleDelete(dev_name_id, id){
  //  $("#tableHostPrintALD").html("<img class=\"system_image\" id=\"file_go\" src=\"images/loader.gif\"><h3>Обновление данных...</h3>");
  var xmlhttp = getXmlHttp(); // Создаём объект XMLHTTP
  xmlhttp.timeout = 60000;
  xmlhttp.ontimeout = function() {
    xmlhttp.abort();
    alert("Данные не получены от сервера (Timeout request).");
  }
  xmlhttp.open('POST', '/policy', true); // Открываем асинхронное соединение
  xmlhttp.setRequestHeader('Cache-Control','max-age=3600');
  xmlhttp.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded');  
  var Resp = "domainpolicy=rmproperty";
  if(dev_name_id.length)
    Resp += "&devname=" + dev_name_id; 
  if(id.length)
    Resp += "&devproperty=" + id;
  xmlhttp.send(Resp);         
  xmlhttp.onreadystatechange = function() { // Ждём ответа от сервера
    if (xmlhttp.readyState == 4) { // Ответ пришёл        
      if(xmlhttp.status == 200) { // Сервер вернул код 200 (что хорошо)          
          //alert(xmlhttp.responseText);                    
;         var out = "";
          var iDivObj = 0;
		  var ListOfNames = JSON.parse(xmlhttp.responseText);
          if(ListOfNames.data.update != "OK"){
            var err_p = "Ошибка удаления правила из политики домена ALD:\r\n";
            err_p +=  ListOfNames.data.update;
            alert(err_p);    
            return 1;
          }
          $(".rule_ald_device_tmp_cols_2").parent().html(function(index, oldText){     
             $(this).remove();
            return oldText;
         });
          $('.rule_tb_ald_list_tmpl tr:last').after("<tr class=\"rule_ald_device_row_next\"><td class=\"rule_ald_device_tmp_cols_2\" id=\"\"></td>\<td class=\"rule_ald_device_tmp_cols_3\" id=\"\"></td><td class=\"rule_ald_device_tmp_cols_4\" id=\"\"></td></tr>").ready(function () {      
            ViewhostlistAldPolicy(dev_name_id);                        
         });    
          return 0;
      }
      else if (xmlhttp.status == 401) { // Нет авторизации
        location.href = '/';
      }
      else {
        //обработка ошибки
        if(this.statusText.length)
          alert('error: ' + this.status ? this.statusText : 'query failed');
        return 2;
      }
    }
  };
    
    
}

function ViewhostlistAldPolicy(PolicyName) {
  //alert('Resources!');  
 // $("#tableHostPrintALD").html("<img class=\"system_image\" id=\"file_go\" src=\"images/loader.gif\"><h3>Обновление данных...</h3>");
  var xmlhttp = getXmlHttp(); // Создаём объект XMLHTTP
  xmlhttp.timeout = 60000;
  xmlhttp.ontimeout = function() {
    xmlhttp.abort();
    alert("Данные не получены от сервера (Timeout request).");
  }
  xmlhttp.open('POST', '/policy', true); // Открываем асинхронное соединение
  xmlhttp.setRequestHeader('Cache-Control','max-age=3600');
  xmlhttp.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded');  
  var Resp = "domainpolicy=viewdev";
  if(PolicyName.length)
    Resp += "&devname=" + PolicyName; 
  xmlhttp.send(Resp);         
  xmlhttp.onreadystatechange = function() { // Ждём ответа от сервера
    if (xmlhttp.readyState == 4) { // Ответ пришёл        
      if(xmlhttp.status == 200) { // Сервер вернул код 200 (что хорошо)          
          //alert(xmlhttp.responseText);                    
;         var out = "";
          var iDivObj = 0;
		  var ListOfNames = JSON.parse(xmlhttp.responseText);
          //alert(xmlhttp.responseText);
          
          if(ListOfNames.state == 0){
            $('button[name=ald_save_user_usb_group]').attr('id',2);            
            
            $('#_ald_usb_name').val(ListOfNames.data[0].name);
           //$('#_ald_usb_id').val(ListOfNames.data[0].serial);
            $('#ald_audit_rule_who_id_').val(ListOfNames.data[0].owner);
            $('#ald_audit_rule_group_id_').val(ListOfNames.data[0].group);
            $('#_ald_usb_info').val(ListOfNames.data[0].info);  
            var HexRule = (ListOfNames.data[0].rule).toString(8); 
            var IntRule = parseInt(HexRule, 8);            
            for(var k = 0; k < 9; k++){                
               $(document.getElementById("ald_discrete_bit_" + k)).prop('checked', ((IntRule&(1<<k)))?true:false);  
            }                                
            $('#ald_s_owner_user_usb').prop('checked', ((ListOfNames.data[0].state == "1") ?true:false));
            $('button[name=add_ald_usb_group]').click();
          }
                    
            var device_tmpl  = $(".rule_tb_ald_list_tmpl").find( $("tbody > tr.rule_ald_device_row_next")).html();
            $(".rule_tb_ald_list_tmpl").find( $("tbody > tr.rule_ald_device_row_next")).remove();
            for(i in ListOfNames.data[0].serial) { 
              $('.rule_tb_ald_list_tmpl tr:last').after("<tr id=\"tr_" + ListOfNames.data[0].serial + "\" class=\"rule_ald_device_row_next\">" + device_tmpl + "</tr>");
              iDivObj += 1;
            }
            TemplateUsbPropUpdateData(ListOfNames);
            

      }
      else if (xmlhttp.status == 401) { // Нет авторизации
        location.href = '/';
      }
      else {
        //обработка ошибки
        if(this.statusText.length)
          alert('error: ' + this.status ? this.statusText : 'query failed');
        return;
      }
    }
  };
}

function AppendhostlistAldPolicy() {
  var xmlhttp = getXmlHttp(); // Создаём объект XMLHTTP
  xmlhttp.timeout = 60000;
  xmlhttp.ontimeout = function() {
    xmlhttp.abort();
    alert("Данные не получены от сервера (Timeout request).");
  }
  xmlhttp.open('POST', '/policy', true); // Открываем асинхронное соединение
  xmlhttp.setRequestHeader('Cache-Control','max-age=3600');
  xmlhttp.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded');  
  var prop_mode = $('button[name=ald_save_user_usb_group]').attr('id');
 
  var Resp="";

  if(prop_mode == 2)
    Resp += "domainpolicy=modrule";
  else
    Resp += "domainpolicy=appendrule";
  
  var DevName="";
  var AllRules="";
  var DevOwner="";
  var DevGroup="";
  var DevDescr="";

  
  DevName =$('#_ald_usb_name').val();
  
  //AllRules=$('#_ald_usb_id').val();
  
   var cnt_group = 0;
   $(".rule_ald_device_tmp_cols_2").html(function(index, oldText){     
         if(cnt_group == 0) AllRules  = oldText;
         if(cnt_group > 0)  AllRules += "|" + oldText;
          cnt_group = cnt_group + 1;  
          return oldText;
   });
   if(cnt_group < 1){
          alert('Для устройства должно быть описано по крайней мере 1 правило.\r\nНе допускается описание устройства без правил.');
          return;
  }
  
  DevOwner=$('#ald_audit_rule_who_id_').find("option:selected").attr('value');
  DevGroup=$('#ald_audit_rule_group_id_').find("option:selected").attr('value');
  DevDescr=$('#_ald_usb_info').val();
  
  var nNewDiscreteVal = 0;
  for(var k = 0; k < 9; k++){      
    if(document.getElementById("ald_discrete_bit_" + k).checked)
      nNewDiscreteVal|=(1<<k);
  }
  var DevMode = (nNewDiscreteVal).toString(8); 
  var DevState=($('#ald_s_owner_user_usb').is(':checked'))?"1":"0"; 
    
  if(DevName.length)
    Resp += "&devname=" + DevName; 
  else{
    alert('Ошибка заполнения формы.\r\nНе указано наименование устройства!');  
     return 1;  
  }
  if(AllRules.length)
    Resp += "&devserial=\"" + AllRules + "\""; 
  else{
    alert('Ошибка заполнения формы.\r\nНе указаны идентификаторы устройства!');  
    return 2;  
  }
  if(DevOwner.length)
    Resp += "&devowner=" + DevOwner; 
  else{
    alert('Ошибка заполнения формы.\r\nНе указан пользователь устройства!');  
    return 3;  
  }
  if(DevGroup.length)
    Resp += "&devgroup=" + DevGroup; 
  else{
    alert('Ошибка заполнения формы.\r\nНе указано группа для устройства!');  
    return 4;  
  }
  if(DevDescr.length)
    Resp += "&devdescr=" + DevDescr; 
  if(DevMode.length)
    Resp += "&devmode=0" + DevMode; 
  else{
    alert('Ошибка заполнения формы.\r\nНе указаны атрибуты доступа устройства!');  
    return 5;  
  }
  if(DevState.length)
    Resp += "&devstate=" + DevState; 
  else
    Resp += "&devstate=1"; 
   
  
  xmlhttp.send(Resp);  
  
  xmlhttp.onreadystatechange = function() { // Ждём ответа от сервера
    if (xmlhttp.readyState == 4) { // Ответ пришёл        
      if(xmlhttp.status == 200) { // Сервер вернул код 200 (что хорошо)          
         // alert(xmlhttp.responseText);                    
;         var out = "";
          var iDivObj = 0;
		  var ListOfNames = JSON.parse(xmlhttp.responseText);
          //alert(xmlhttp.responseText);
          if(ListOfNames.data.update != "OK"){
              if(ListOfNames.data.update != undefined){
                var err_p = "Ошибка добавления устройства в политику домена ALD:\r\n";
                err_p +=  ListOfNames.data.update;
                alert(err_p);    
              }
          }
          if(ListOfNames.data.update == "OK"){
            $("#tableHostPrintALD").html("<img class=\"system_image\" id=\"file_go\" src=\"images/loader.gif\"><h3>Обновление данных...</h3>");
            hostlistAldPolicy();                
          }
          return 0;

      }
      else if (xmlhttp.status == 401) { // Нет авторизации
        location.href = '/';
      }
      else {
        //обработка ошибки
        if(this.statusText.length)
          alert('error: ' + this.status ? this.statusText : 'query failed');
        return 6;
      }
    }
  };
}


function CDROMActionList(mHost, mDevice) {
  //alert('Resources!');  
  $("#tableHostPrint").html("<img class=\"system_image\" id=\"file_go\" src=\"images/loader.gif\"><h3>Обновление данных конфигурации...</h3>");
  var xmlhttp = getXmlHttp(); // Создаём объект XMLHTTP
  xmlhttp.timeout = 60000;
  xmlhttp.ontimeout = function() {
    xmlhttp.abort();
    alert("Данные не получены от сервера (Timeout request).");
  }
  xmlhttp.open('POST', '/action', true); // Открываем асинхронное соединение
  xmlhttp.setRequestHeader('Cache-Control','max-age=3600');
  xmlhttp.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded');  
  var Resp = "resourcebyhost=gropdevlist";
  if(mHost.length)
    Resp += "&resourcehost=" + mHost; 
  if(mDevice.length)
    Resp += "&device=" + mDevice; 
  xmlhttp.send(Resp);   
  xmlhttp.onreadystatechange = function() { // Ждём ответа от сервера
    if (xmlhttp.readyState == 4) { // Ответ пришёл        
      if(xmlhttp.status == 200) { // Сервер вернул код 200 (что хорошо)          
          //alert(xmlhttp.responseText);                    
;         var out = "";
          var iDivObj = 0;
		  var ListOfNames = JSON.parse(xmlhttp.responseText);
          //alert(xmlhttp.responseText);
          
          $('#tableHostPrint').html("");                      
          $('#tableHostPrint').load('html/cdrom_tmp_devices.html', '.group_list_all_div', function(){              	 
		    var NamesData   = ListOfNames.data;                                
            /*var file_row_tmpl  = $(".tb_group_dev_list_tmpl").find( $("tbody > tr.device_row_next")).html(); 
            $(".tb_group_dev_list_tmpl").find( $("tbody > tr.device_row_next")).remove();
            */
            $('.header1').html("<span>Параметры конфигурации: <p class=\"resource_host\">" + mHost + "</p></span>");
            $('.header2').html("<span id=\"glowtext\" onclick=\"CheckLoader(this);hostlistAld();\"><img class=\"system_image\" id=\"back_go\"  onclick=\"Check(this);\"  src=\"images/back.png\">Вернуться к списку устройств</span>");
            /*
            for(i in ListOfNames.data) {
                $('.tb_group_dev_list_tmpl tr:last').after("<tr>" + file_row_tmpl + "</tr>");
                iDivObj += 1;
              
            }     
            */
            LoadTemplateDataDevCD(mHost, ListOfNames);
            LocalFindDevPublicFstab(mHost);
          });  
          //SCP
          HostTouchStripts(mHost);
      }
      else if (xmlhttp.status == 401) { // Нет авторизации
        location.href = '/';
      }
      else {
        //обработка ошибки
        if(this.statusText.length)
          alert('error: ' + this.status ? this.statusText : 'query failed');
        return;
      }
    }
  };
}
 
  
 
function ProtectedActionList(mHost, mPath, mState) { 
  if(mState != 1)
    $(".backup_dialog").html("<img class=\"system_image\" id=\"file_go\" src=\"images/loader.gif\"><h3>Обновление данных...</h3>");
  var xmlhttp = getXmlHttp(); // Создаём объект XMLHTTP
  xmlhttp.timeout = 60000;
  xmlhttp.ontimeout = function() {
    xmlhttp.abort();
    alert("Данные не получены от сервера (Timeout request).");
  }
  xmlhttp.open('POST', '/action', true); // Открываем асинхронное соединение
  xmlhttp.setRequestHeader('Cache-Control','max-age=3600');
  xmlhttp.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded');  
  var Resp = "resourcebyhost=backupssavelist";
  if(mHost.length)
    Resp += "&resourcehost=" + mHost; 
  if(mPath.length)
    Resp += "&resourcepath=" + mPath; 
  xmlhttp.send(Resp);   
  xmlhttp.onreadystatechange = function() { // Ждём ответа от сервера
    if (xmlhttp.readyState == 4) { // Ответ пришёл        
      if(xmlhttp.status == 200) { // Сервер вернул код 200 (что хорошо)          
          //alert(xmlhttp.responseText);                    
;         var out = "";
          var iDivObj = 0;
		  var ListOfNames = JSON.parse(xmlhttp.responseText);    
          if(mState == 1 && update_object == xmlhttp.responseText){
            update_object = xmlhttp.responseText;
            return;
          } 
          if(mState == 1)
            update_object = xmlhttp.responseText;
          else 
            update_object = 0;
          $('.backup_dialog').html("");                      
          $('.backup_dialog').load('html/protected_tmp_devices.html', '.host_list_backup_div', function(){              	 
		    var NamesData      = ListOfNames.data;               
            var file_row_tmpl  = $(".protected_tb_hosts_list_tmpl").find( $("tbody > tr.protected_device_row_next")).html(); 
            $(".protected_tb_hosts_list_tmpl").find( $("tbody > tr.protected_device_row_next")).remove();    
            $('.path_save_pro').text(mPath);
            for(i in ListOfNames.data) {
                $('.protected_tb_hosts_list_tmpl tr:last').after("<tr>" + file_row_tmpl + "</tr>");
                iDivObj += 1;              
            }                  
             $(".protected_device_tmp_cols_1").html(function(index, oldText){ 
                oldText = "<span id=\"glowtext\">" + ListOfNames.data[index].arc_file + "</span>";    
                 return oldText;
             }); 
             $(".protected_device_tmp_cols_2").html(function(index, oldText){ 
                if(ListOfNames.data[index].id_state_process == "-1" ||   ListOfNames.data[index].id_state_process == "")
                  oldText = "<img class=\"system_image\" id=\"file_go\" src=\"images/loader.gif\">";    
                else if(ListOfNames.data[index].id_state_process == "0")
                  oldText = "<span id=\"glowtext\">Выполнено</span>";                 
                else if(ListOfNames.data[index].id_state_process == "4")
                  oldText = "<span id=\"glowtext\">Ошибка отправки FTP</span>";                    
                else if(ListOfNames.data[index].id_state_process == "4")
                  oldText = "<span id=\"glowtext\">Ошибка создания архива</span>";
                else
                  oldText = "<span id=\"glowtext\">Ошибка выполнения операции</span>";
                 return oldText;
             }); 
             $(".protected_device_tmp_cols_3").html(function(index, oldText){ 
                oldText = "<span id=\"glowtext\">" + ListOfNames.data[index].time_start + "</span>";    
                 return oldText;
             }); 
             $(".protected_device_tmp_cols_4").html(function(index, oldText){ 
                 if(ListOfNames.data[index].id_state_process != "-1")
             
                  oldText = "<span id=\"glowtext\">" + ListOfNames.data[index].time_stop + "</span>";
    
                return oldText;
             });
             
            $('button[name=save_protected_folder]').click(function() {          
                ProtectedActionListOperation(mHost, mPath, 1, ''); 
            });
             $('button[name=save_protected_folder_upadte]').click(function() {          
                ProtectedActionList(mHost, mPath, 0); 
            });
            UpdateBackupTimer(mHost, mPath); 
             
      });   
          
      }
      else if (xmlhttp.status == 401) { // Нет авторизации
        location.href = '/';
      }
      else {
        //обработка ошибки
        if(this.statusText.length)
          alert('error: ' + this.status ? this.statusText : 'query failed');
        return;
      }
    }
  };
}
 
 
function UpdateBackupTimer(mHost, mPath){
  var object = this;
  this.start = function() {
    //alert("start");
    timer = setInterval(object.tick, 10000);
  }
  this.stop = function() {
    clearInterval(timer);
    timer = 0;
  }
  this.tick = function() {
    //alert("tick");
    if($('#table_backup_dialog').dialog('isOpen') == false){ 
      update_object = 0;
      //alert("stop");
      object.stop();         
    }
    else{  
      //alert("update");
      var mHostCur =  $('.resource_host').text(); 
      var mPathCur =  $('.path_save_pro').text();
      ProtectedActionList(mHostCur, mPathCur, 1);        
    }
  }  
  if(update_object == 0){
    this.stop();
    this.start();
  }
  $('#table_backup_dialog').on('dialogclose', function(event) {
     //alert('closed');
     update_object      = 0;
     object.stop();  
  }); 
}

 
function ProtectedActionListOperation(mHost, mPath, mOperation, mRec) { 
  $(".backup_dialog").html("<img class=\"system_image\" id=\"file_go\" src=\"images/loader.gif\"><h3>Обновление данных...</h3>");
  var xmlhttp = getXmlHttp(); // Создаём объект XMLHTTP
  xmlhttp.timeout = 60000;
  xmlhttp.ontimeout = function() {
    xmlhttp.abort();
    alert("Данные не получены от сервера (Timeout request).");
  }
  xmlhttp.open('POST', '/action', true); // Открываем асинхронное соединение
  xmlhttp.setRequestHeader('Cache-Control','max-age=3600');
  xmlhttp.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded');  
  var Resp = "resourcebyhost=saveresourceDev";
  if(mHost.length)
    Resp += "&resourcehost=" + mHost; 
  if(mPath.length)
    Resp += "&resourcepath=" + mPath; 
  if(mOperation.length)
    Resp += "&operation=" + mOperation;
  if(mRec.length && mRec == 'R')
    Resp += "&recursive=R";  
  xmlhttp.send(Resp);   
  xmlhttp.onreadystatechange = function() { // Ждём ответа от сервера
    if (xmlhttp.readyState == 4) { // Ответ пришёл        
      if(xmlhttp.status == 200) { // Сервер вернул код 200 (что хорошо)          
          //alert(xmlhttp.responseText);                    
;         var out = "";
          var iDivObj = 0;
		  var ListOfNames = JSON.parse(xmlhttp.responseText);
          
          $('.backup_dialog').html(""); 
          if(ListOfNames.data.update == "1"){              
            ProtectedActionList(mHost, mPath, 0); 
            /*alert('Задача сохранения резервируемого ресурса: ' +  mHost + '  '+ mPath + ' добавлена.'); */
          }
          else{
           alert('Не удалось выполнить задачу сохранения резервируемого ресурса: ' +  mHost + '  ' + mPath);   
         }

      }
      else if (xmlhttp.status == 401) { // Нет авторизации
        location.href = '/';
      }
      else {
        //обработка ошибки
        if(this.statusText.length)
          alert('error: ' + this.status ? this.statusText : 'query failed');
        return;
      }
    }
  };
}
  

 
 
 
 function ModifyCDROMActionList(mHost, mDevice, mUser, mRule) {  
  $("#tableHostPrint").html("<img class=\"system_image\" id=\"file_go\" src=\"images/loader.gif\"><h3>Обновление данных конфигурации...</h3>");
  var xmlhttp = getXmlHttp(); // Создаём объект XMLHTTP
  xmlhttp.timeout = 60000;;
  xmlhttp.ontimeout = function() {
    xmlhttp.abort();
    alert("Данные не получены от сервера (Timeout request).");
  }
  xmlhttp.open('POST', '/action', true); // Открываем асинхронное соединение
  xmlhttp.setRequestHeader('Cache-Control','max-age=3600');
  xmlhttp.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded');  
  var Resp = "resourcebyhost=setusertodev";
  if(mHost.length)
    Resp += "&resourcehost=" + mHost; ;
  if(mDevice.length)
    Resp += "&device=" + mDevice; 
  if(mUser.length)
    Resp += "&user_dev=" + mUser;
  if(mRule.length)
    Resp += "&rule=" + mRule; 

  xmlhttp.send(Resp);   
  xmlhttp.onreadystatechange = function() { // Ждём ответа от сервера
    if (xmlhttp.readyState == 4) { // Ответ пришёл        
      if(xmlhttp.status == 200) { // Сервер вернул код 200 (что хорошо)                             
;         var out = "";
          var iDivObj = 0;
		  var ListOfNames = JSON.parse(xmlhttp.responseText);
          //alert(xmlhttp.responseText);
          if(ListOfNames.data.update == "OK"){
             CDROMActionList(mHost, mDevice);             
          }                        
          else
            alert('Не удалось обновить данные конфигурации.');

      }
      else if (xmlhttp.status == 401) { // Нет авторизации
        location.href = '/';
      }
      else {
        //обработка ошибки
        if(this.statusText.length)
          alert('error: ' + this.status ? this.statusText : 'query failed');
        return;
      }
    }
  };
}



function RuleDomainResourceLocalObjectsSelect(mSelect) {
  var HttpDataParams = "hostsAld=hostlist";
  var xmlhttp = getXmlHttp(); // Создаём объект XMLHTTP
  xmlhttp.timeout = 60000;
  xmlhttp.ontimeout = function() {
    xmlhttp.abort();
    alert("Данные не получены от сервера (Timeout request).");
  }  
  xmlhttp.open('POST', '/ald', true); // Открываем асинхронное соединение
  xmlhttp.setRequestHeader('Cache-Control','max-age=3600');
  xmlhttp.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded');  

  xmlhttp.send(HttpDataParams);       
  xmlhttp.onreadystatechange = function() { // Ждём ответа от сервера
    if (xmlhttp.readyState == 4) { // Ответ пришёл        
      if(xmlhttp.status == 200) { // Сервер вернул код 200 (что хорошо)                    
          //alert(xmlhttp.responseText);  
          var ListOfNames = JSON.parse(xmlhttp.responseText);		            
          if(ListOfNames.state != 0){
            alert("Ошибка получения данных от сервера:<br><h3>" + ListOfNames.data.update + "</h3>"); 
            return;
          }  
          $(mSelect).html("");                    
          for(i in ListOfNames.data) {  
            var NamesData   = ListOfNames.data[i];      
            $(mSelect).append("<option value=\"" +  ListOfNames.data[i].host_hame +"\">"+  ListOfNames.data[i].host_hame +"</option>").ready(function () {      
            });                        
         }
         DomainFindDevRuleActionList($(mSelect).val()); 
      }
      else if (xmlhttp.status == 401) { // Нет авторизации
        location.href = '/';
      }
      else {
        //обработка ошибки
        if(this.statusText.length)
          alert('error: ' + this.status ? this.statusText : 'query failed');
        return;
      }
    }
  };
}




function LocalFindDevPublicFstab(mHost) {
  //alert("CD");  
  var xmlhttp = getXmlHttp(); // Создаём объект XMLHTTP
  xmlhttp.timeout = 60000;
  xmlhttp.ontimeout = function() {
    xmlhttp.abort();
    //alert("Данные не получены от сервера (Timeout request).");
  }
  xmlhttp.open('POST', '/action', true); // Открываем асинхронное соединение
  xmlhttp.setRequestHeader('Cache-Control','max-age=3600');
  xmlhttp.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded');  
  var Resp = "resourcebyhost=fstabpubliclist";
  Resp += "&resourcehost=" + mHost; 

  xmlhttp.send(Resp);   
  xmlhttp.onreadystatechange = function() { // Ждём ответа от сервера
    if (xmlhttp.readyState == 4) { // Ответ пришёл        
      if(xmlhttp.status == 200) { // Сервер вернул код 200 (что хорошо)          
          //alert(xmlhttp.responseText);                    
;         var out = "";
          var iDivObj = 0;
		  var ListOfNames = JSON.parse(xmlhttp.responseText);         
          for(i in ListOfNames.data) { 
             out += ListOfNames.data[i] + "\r\n";    
             iDivObj++;
          }
         if(iDivObj > 0){
           $('#fstab_storage').prop('checked',true); 
           //var resIs = "В файле /etc/fstab на хосте " + mHost + " установлены разрешения на монтирование\
           //cd/dvd носителей  для непривилегированных пользователей (user).\r\nЗапретить монтирование для непривилегированных пользователей (рекомендуется)?\r\n" + out;
           //var isYes = confirm(resIs);
           //if(isYes){
           //  LocalFixDevPublicFstab(mHost);
           //}             
         } 
         else
           $('#fstab_storage').prop('checked',false); 
         
        $('#fstab_storage').change(function(){ 
          var host =  $('.resource_host').text();                
          if($(this).is(':checked'))
            LocalFixDevPublicFstab(mHost, 1);
          else
            LocalFixDevPublicFstab(mHost, 0);
        });  
          
      }
      else if (xmlhttp.status == 401) { // Нет авторизации
        location.href = '/';
      }
      else {
        //обработка ошибки
        if(this.statusText.length)
          alert('error: ' + this.status ? this.statusText : 'query failed');
        return;
      }
    }
  };
}



function LocalFixDevPublicFstab(mHost, OnOff) {
  //alert("CD");  
  var xmlhttp = getXmlHttp(); // Создаём объект XMLHTTP
  xmlhttp.timeout = 60000;
  xmlhttp.ontimeout = function() {
    xmlhttp.abort();
    //alert("Данные не получены от сервера (Timeout request).");
  }
  xmlhttp.open('POST', '/action', true); // Открываем асинхронное соединение
  xmlhttp.setRequestHeader('Cache-Control','max-age=3600');
  xmlhttp.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded');  
  var Resp = "resourcebyhost=fstab_public_touch";
  Resp += "&resourcehost=" + mHost; 
  Resp += "&state=" + OnOff;

  xmlhttp.send(Resp);   
  xmlhttp.onreadystatechange = function() { // Ждём ответа от сервера
    if (xmlhttp.readyState == 4) { // Ответ пришёл        
      if(xmlhttp.status == 200) { // Сервер вернул код 200 (что хорошо)          
          //alert(xmlhttp.responseText);                    
;         var out = "";
          var iDivObj = 0;
		  var ListOfNames = JSON.parse(xmlhttp.responseText);         
          if(ListOfNames.data.update == "OK"){
             out += "Файл /etc/fstab на хосте " + mHost + " отредактирован успешно."; 
          }
          else
           out += "Не удалось отредактировать файл /etc/fstab на хосте " + mHost + ".\r\n" + ListOfNames.data.update; 
          if(out.length)
            alert(out);
          
      }
      else if (xmlhttp.status == 401) { // Нет авторизации
        location.href = '/';
      }
      else {
        //обработка ошибки
        if(this.statusText.length)
          alert('error: ' + this.status ? this.statusText : 'query failed');
        return;
      }
    }
  };
}


function DomainFindDevRuleActionAutorized(mHost, mDev) {
  //alert(mDev);  
  var xmlhttp = getXmlHttp(); // Создаём объект XMLHTTP
  xmlhttp.timeout = 60000;
  xmlhttp.ontimeout = function() {
    xmlhttp.abort();
    alert("Данные не получены от сервера (Timeout request).");
  }
  xmlhttp.open('POST', '/policy', true); // Открываем асинхронное соединение
  xmlhttp.setRequestHeader('Cache-Control','max-age=3600');
  xmlhttp.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded');  
  var Resp = "domainpolicy=autorizeproperty";
  if(mHost.length)
    Resp += "&devhost=" + mHost; 
  if(mDev.length)
    Resp += "&devname=" + mDev; 

  xmlhttp.send(Resp);   
  xmlhttp.onreadystatechange = function() { // Ждём ответа от сервера
    if (xmlhttp.readyState == 4) { // Ответ пришёл        
      if(xmlhttp.status == 200) { // Сервер вернул код 200 (что хорошо)          
          //alert(xmlhttp.responseText);                    
;         var out = "";
          var iDivObj = 0;
		  var ListOfNames = JSON.parse(xmlhttp.responseText);            	 
      }
      else if (xmlhttp.status == 401) { // Нет авторизации
        location.href = '/';
      }
      else {
        //обработка ошибки
        if(this.statusText.length)
          alert('error: ' + this.status ? this.statusText : 'query failed');
        return;
      }
    }
  };
}



function DomainFindDevAlive() {
  //alert("mDev");  
  var xmlhttp = getXmlHttp(); // Создаём объект XMLHTTP
  xmlhttp.timeout = 60000;
  xmlhttp.ontimeout = function() {
    xmlhttp.abort();
    alert("Данные не получены от сервера (Timeout request).");
  }
  xmlhttp.open('POST', '/action', true); // Открываем асинхронное соединение
  xmlhttp.setRequestHeader('Cache-Control','max-age=3600');
  xmlhttp.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded');  
  var Resp = "resourcebyhost=alive_host";

  xmlhttp.send(Resp);   
  xmlhttp.onreadystatechange = function() { // Ждём ответа от сервера
    if (xmlhttp.readyState == 4) { // Ответ пришёл        
      if(xmlhttp.status == 200) { // Сервер вернул код 200 (что хорошо)          
          //alert(xmlhttp.responseText);                    
;         var out = "";
          var iDivObj = 0;
		  var ListOfNames = JSON.parse(xmlhttp.responseText);  
          
          var test     = $('#tab_devices').is(':checked')?"1":"0"; 
          var mn_page  = $('button[name=dev_main_upd]').prop('onclick');
    
          if(test == 0 && mn_page == undefined)
            return;                                  
          
          $(".device_tmp_cols_2").parent().html(function(index, oldText){                          
            var dev_id = $(this).find('.device_tmp_cols_2').text();                    
            if(dev_id != undefined && dev_id != ""){
              //-------------------------------------  
              for(i in ListOfNames.data) { 
                var devName     = ListOfNames.data[i].hostname;                
                if(devName != dev_id) continue;
                var time_alive  = ((ListOfNames.data[i].time_stop == undefined)?"":ListOfNames.data[i].time_stop);
                var alive_txt   = ((ListOfNames.data[i].id_state == 1)?"<span><font color=\"green\">устройство готово к работе</font></span>":"<span><font color=\"red\">устройство недоступно</font></span>");
                alive_txt = alive_txt + "<br>" +  formatDate(time_alive);
                if(devName == dev_id && (ListOfNames.data[i].id_state == 1 || ListOfNames.data[i].id_state == 0)){
                  $(this).find('.device_tmp_cols_6').html(alive_txt);              
                }
                //else
                //  $(this).find('.device_tmp_cols_6').html(""); 
              } 
            }
            //-------------------------------------
            //return oldText;
          });
          
          
        $('.device_tmp_cols_7').html(function(index, oldText){
          var host = $(this).attr('id');         
          if($(this).parent().find('.device_tmp_cols_6 font').text() == 'устройство готово к работе')
            oldText    = "<span  id=\"glowtext\" onclick=\"hostAction('" + host + "','/');\"><img class=\"system_image\" src=\"images/settings-icon.png\" onclick=\"Check(this);\"></span>";
          else
            oldText  = "<span  id=\"glowtext\"><img class=\"system_image\" src=\"images/delete.png\"></span>";
          return oldText;
        });;
        $('.device_tmp_cols_8').html(function(index, oldText){ 
          var host = $(this).attr('id');        
          if($(this).parent().find('.device_tmp_cols_6 font').text() == 'устройство готово к работе')
            oldText    = "<span id=\"glowtext\" onclick=\"AuditAction('" + host + "','/','');\"><img class=\"system_image\" //src=\"images/settings-icon.png\" onclick=\"Check(this);\"></span>";
          else
            oldText  = "<span  id=\"glowtext\"><img class=\"system_image\" src=\"images/delete.png\"></span>";          
          return oldText;
        });      
        $('.device_tmp_cols_9').html(function(index, oldText){ 
          var host = $(this).attr('id');
          if($(this).parent().find('.device_tmp_cols_6 font').text() == 'устройство готово к работе')
            oldText    = "<span id=\"glowtext\" onclick=\"CDROMActionList('" + host  + "','cdrom');\"><img class=\"system_image\" src=\"images/settings-icon.png\" onclick=\"Check(this);\"></span>";
          else
            oldText  = "<span  id=\"glowtext\"><img class=\"system_image\" src=\"images/delete.png\"></span>";          
          return oldText;
        });   
        $('.device_tmp_cols_10').html(function(index, oldText){ 
          var host = $(this).attr('id');
          if($(this).parent().find('.device_tmp_cols_6 font').text() == 'устройство готово к работе')
            oldText    = "<span id=\"glowtext\" onclick=\"SaveResourceAction('" + host  + "');\"><img class=\"system_image\" src=\"images/settings-icon.png\" onclick=\"Check(this);\"></span>";
          else
            oldText  = "<span  id=\"glowtext\"><img class=\"system_image\" src=\"images/delete.png\"></span>";          
          return oldText;
        }); 
        
       
          /*
          for(i in ListOfNames.data) { 
            var devName    = ListOfNames.data[i].hostname;
            var time_alive = ((ListOfNames.data[i].time_stop == undefined)?"":ListOfNames.data[i].time_stop);
            var alive_txt = ((ListOfNames.data[i].id_state == 1)?"<span><font color=\"green\">устройство готово к работе</font></span>":"<span><font color=\"red\">устройство недоступно</font></span>");
            alive_txt = alive_txt + "<br>" +  time_alive;
            if(devName != undefined && (ListOfNames.data[i].id_state == 1 || ListOfNames.data[i].id_state == 0)){
                $(document.getElementById(devName.toString())).html(alive_txt);              
            }
          } 
          */
        }
        var table = $('.tb_hosts_list_tml').DataTable();
      }
      else if (xmlhttp.status == 401) { // Нет авторизации
        location.href = '/';
      }
      else {
       //обработка ошибки
       // if(this.statusText.length)
       //alert('error: ' + this.status ? this.statusText : 'query failed');
        return;
      }
    }
  }



  
function UdevLocalClear(mHost) {
  var xmlhttp = getXmlHttp(); // Создаём объект XMLHTTP
  xmlhttp.timeout = 60000;
  xmlhttp.ontimeout = function() {
    xmlhttp.abort();
    alert("Данные не получены от сервера (Timeout request).");
  }
  xmlhttp.open('POST', '/action', true); // Открываем асинхронное соединение
  xmlhttp.setRequestHeader('Cache-Control','max-age=3600');
  xmlhttp.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded');  
  var Resp = "resourcebyhost=delete_udev_touch";
  Resp += "&resourcehost=" + mHost; ;
  xmlhttp.send(Resp);   
  
  xmlhttp.onreadystatechange = function() { // Ждём ответа от сервера
    if (xmlhttp.readyState == 4) { // Ответ пришёл        
      if(xmlhttp.status == 200) { // Сервер вернул код 200 (что хорошо)          
            //alert(xmlhttp.responseText);                    
;           var out = "";
            var iDivObj = 0;
		    var ListOfNames = JSON.parse(xmlhttp.responseText);  
            USBRulesActionListNeedTouch(mHost, '','', 0);
         }
      }
      else if (xmlhttp.status == 401) { // Нет авторизации
        location.href = '/';
      }
      else {
       //обработка ошибки
       // if(this.statusText.length)
       //alert('error: ' + this.status ? this.statusText : 'query failed');
        return;
      }
    }
  }

  
  
function HostTouchStripts(mHost) {
  var xmlhttp = getXmlHttp(); // Создаём объект XMLHTTP
  xmlhttp.timeout = 60000;
  xmlhttp.ontimeout = function() {
    xmlhttp.abort();
    alert("Данные не получены от сервера (Timeout request).");
  }
  xmlhttp.open('POST', '/action', true); // Открываем асинхронное соединение
  xmlhttp.setRequestHeader('Cache-Control','max-age=3600');
  xmlhttp.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded');  
  var Resp = "resourcebyhost=dev_scp";
  Resp += "&resourcehost=" + mHost; ;
  xmlhttp.send(Resp);   
  
  xmlhttp.onreadystatechange = function() { // Ждём ответа от сервера
    if (xmlhttp.readyState == 4) { // Ответ пришёл        
      if(xmlhttp.status == 200) { // Сервер вернул код 200 (что хорошо)          
            //alert(xmlhttp.responseText);                    
;           var out = "";
            var iDivObj = 0;
		    var ListOfNames = JSON.parse(xmlhttp.responseText);  
         }
      }
      else if (xmlhttp.status == 401) { // Нет авторизации
        location.href = '/';
      }
      else {
       //обработка ошибки
       // if(this.statusText.length)
       //alert('error: ' + this.status ? this.statusText : 'query failed');
        return;
      }
    }
  }
  
  


function DomainFindDevRuleActionList(mHost) {
  //alert(mHost);  
  var xmlhttp = getXmlHttp(); // Создаём объект XMLHTTP
  xmlhttp.timeout = 60000;
  xmlhttp.ontimeout = function() {
    xmlhttp.abort();
    alert("Данные не получены от сервера (Timeout request).");
  }
  xmlhttp.open('POST', '/policy', true); // Открываем асинхронное соединение
  xmlhttp.setRequestHeader('Cache-Control','max-age=3600');
  xmlhttp.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded');  
  var Resp = "domainpolicy=list_disk_serial";
  if(mHost.length)
    Resp += "&resourcehost=" + mHost; 
  xmlhttp.send(Resp);   
  xmlhttp.onreadystatechange = function() { // Ждём ответа от сервера
    if (xmlhttp.readyState == 4) { // Ответ пришёл        
      if(xmlhttp.status == 200) { // Сервер вернул код 200 (что хорошо)          
          //alert(xmlhttp.responseText);                    
;         var out = "";
          var iDivObj = 0;
		  var ListOfNames = JSON.parse(xmlhttp.responseText);            	 
          var NamesData   = ListOfNames.data;                                
          $(".find_rule_ald_device_row_next").html(function(index, oldText){     
              $(this).remove();
               return oldText;
          });  
          for(i in ListOfNames.data) {                                                  
            $('.find_rule_tb_ald_list_tmpl tr:last').after("<tr class=\"find_rule_ald_device_row_next\"><td class=\"find_rule_ald_device_tmp_cols_2\" id=\"" + ListOfNames.data[i].drive +"\"><img class=\"system_image\" src=\"images/usb.png\">" + ListOfNames.data[i].drive + "</td>\<td class=\"find_rule_ald_device_tmp_cols_3\" id=\"\">" + ListOfNames.data[i].serial + "</td><td class=\"find_rule_ald_device_tmp_cols_4\"><span id=\"glowtext\"><img drive=\"" + ListOfNames.data[i].drive +"\" name=\"findruleupdateudev\" class=\"system_image\" id=\"" + ListOfNames.data[i].serial + "\" src=\"images/plus.png\"></span></td></tr>").ready(function () {                
            });   
            iDivObj += 1;              
          }
          $('img[name=findruleupdateudev]').click(function() { 
            var prop_ = $(this).attr('id');
            var drive = $(this).attr('drive');
            if(prop_ !=  ''){
              $('#_ald_usb_id').val(prop_);                                                 
              $('#find_rule_host_list_all_div').dialog('close');
              $('button[name=rule_ald_save_user_usb_group]').click();
              //DomainFindDevRuleActionAutorized(mHost, drive); 
            }                     
          }); 
          if(iDivObj == 0)
            $('.find_rule_tb_ald_list_tmpl tr:last').after("<tr class=\"find_rule_ald_device_row_next\"><td colspan=\"3\">Не найдены подключенные устройства</td></tr>").ready(function () {});
      }
      else if (xmlhttp.status == 401) { // Нет авторизации
        location.href = '/';
      }
      else {
        //обработка ошибки
        if(this.statusText.length)
          alert('error: ' + this.status ? this.statusText : 'query failed');
        return;
      }
    }
  };
}
 



