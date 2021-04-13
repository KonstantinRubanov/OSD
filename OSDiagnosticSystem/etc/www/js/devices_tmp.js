var g_checked_rec = false;

$(document).ready(function(){
   hostlistAld();
});

function LoadTemplateContentAuditFilesMain(mHost, mAction, ListOfNames, mConfig){  
  $(".auditblock_labels").html("");
  var host =  $('.resource_host').text();
  var cnt = 0;  
  var HeaderList = "";
  if(mConfig == "") 
    $('#header_block').append("<span id=\"glowtext\" onclick=\"CheckLoader(this);hostlistAld();\"><img class=\"system_image\" src=\"images/pc_icons.png\" onclick=\"Check(this);\">Вернуться к списку устройств</span>").ready(function () {      
  });
  else
    $('#header_block').append("<span id=\"glowtext\" onclick=\"CheckLoader(this);AuditAction('" + mHost + "','/','');\"><img  class=\"system_image\" src=\"images/settings-icon.png\" onclick=\"Check(this);\">Вернуться к списку аудита " + mHost + "</span><br>").ready(function () {      
  });
  var rec_check = (g_checked_rec == true)?"checked":"";
  if(mConfig != "") 
    $('#header_block').append("<span><p id=\"controls_rec\"><input type=\"checkbox\" id=\"s_recursive\" value=\"recursive\" " + rec_check + "> Применить рекурсию</p></span>").ready(function () {      
  });  
  
  if(mConfig == "")
  $('#pathtable tr:last').after("<tr><th>Ресурс</th><th>Настройка</th><th>Журнал</th><th></th></tr>").ready(function () {      
  });
  for(k in ListOfNames.data) { 
    alert(ListOfNames.data[k].res_hame);
    if(mConfig == ""){              
      $('#pathtable tr:last').after("<tr><td>" + ListOfNames.data[k].res_hame + "</td><td><span id=\"glowtext\" onclick=\"CheckLoader(this);AuditAction(\'" + mHost + "\',\'" + ListOfNames.data[k].res_hame + "/\',\'" + ListOfNames.data[k].res_attr + "\');\"><img class=\"system_image\" src=\"images/settings-icon.png\" onclick=\"Check(this);\"></span></td><td><span id=\"glowtext\" onclick=\"AuditJournalAction(\'" + mHost + "\',\'" + ListOfNames.data[k].res_hame + "\');\"><img class=\"system_image\" src=\"images/state_write.png\"></span></td><td><span id=\"glowtext\" onclick=\"AuditActionAdd(\'" + mHost + "\',\'" + ListOfNames.data[k].res_hame + "\',\'-\',true,\'\');\"><img class=\"rule_remove_audit\" id=\"#remove_audit\" name=\"rem_audit\" src=\"images/delete_icon.png\"/></span></td>< tr>").ready(function () {      
      });;
    }
    else{
      $('#pathtable tr:last').after("<tr><td></td><td><td><td></td></tr>").ready(function () {      
       });                
    }
    cnt++;
  } 
}

function LoadTemplateContentAuditFiles(mHost, mAction){  
  $(".auditblock_labels .rules_hidden_dir").html("");
  var button_audit_add = $(".auditblock_labels .rules_audit_button").html();
  $(".auditblock_labels .rules_audit_button").html("");
  var host =  $('.resource_host').text();
  $('.dir_form_select').append(button_audit_add).ready(function () {
  });   
  DirAction(host,"/");
}

function LoadTemplateContentAuditList(mHost, mAction, mListOfAudit,block) {
  var header = $(".auditblock_labels .rules_audit_header").find( $("tbody > tr.rule_header_th")).html();
  var rule   = $(".auditblock_labels .rules_audit_header").find( $("tbody > tr.rule_row_next")).html();  
  var block  = $(".auditblock_labels .add_audit_div");  
  var block_bt  = $(".auditblock_labels .add_audit_bt_mod_div"); 
  $(".auditblock_labels").html("");  
  var cnt = 0;  
  $('#pathtable tr:last').after("<tr>" + header +"</tr>");
  if(mListOfAudit != "")
  for(k in  mListOfAudit){  
    $('#pathtable tr:last').after("<tr>" + rule + "</tr>");
    cnt++;
  }
  $('#pathtable').after(block_bt);
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
  //if(cnt > 0){; 
    $('.auditblock_labels').append(block).ready(function () {
      UpdateRulesEditForm ();
    });  

 // }
}

function hostlistAld() {
  //alert('hostlist');  
 $("#ResourceCurrent").html("<img class=\"system_image\" id=\"file_go\"><h3>Обновление списка устройств...</h3>");
  var xmlhttp = getXmlHttp(); // Создаём объект XMLHTTP
  xmlhttp.timeout = 30000;
  xmlhttp.ontimeout = function() {
    xmlhttp.abort();
    alert("Данные не получены от сервера (Timeout request).");
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
            var device_tmpl  = $(".tb_hosts_list_tmpl").find( $("tbody > tr.device_row_next")).html();
            $(".tb_hosts_list_tmpl").find( $("tbody > tr.device_row_next")).remove();
            for(i in ListOfNames.data) { 
              $('.tb_hosts_list_tmpl tr:last').after("<tr>" + device_tmpl + "</tr>");
              iDivObj += 1;
            }
            LoadTemplateContentHosts(ListOfNames);            
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

 
function take_obj(mObj){
  $(document.getElementById(mObj)).slideToggle("slow");
}


function DirAction(mHost, mAction, Frame) {
  //alert('hostResources!');  
  $(".rules_hidden_dir").html("<img class=\"system_image\" id=\"file_go\"><h3>Обновление списка файлов...</h3>")
  var xmlhttp = getXmlHttp(); // Создаём объект XMLHTTP
  xmlhttp.timeout = 30000;
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
            LoadTemplateContentDir(mHost, url, ListOfNames); 
            UpdateRulesEditForm ();
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
  $("#ResourceCurrent").html("<img class=\"system_image\" id=\"file_go\"><h3>Обновление списка файлов...</h3>");
  var xmlhttp = getXmlHttp(); // Создаём объект XMLHTTP
  xmlhttp.timeout = 30000;
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
          var HeaderList = "<br><span id=\"glowtext\" onclick=\"CheckLoader(this);hostlistAld();\"><img class=\"system_image\"  src=\"images/pc_icons.png\">Вернуться к списку устройств</span>";
          
          
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



function AuditAction(mHost, mAction, mConfig) {
  //alert('hostResources!');
  var xmlhttp = getXmlHttp(); // Создаём объект XMLHTTP
  xmlhttp.timeout = 30000;
  xmlhttp.ontimeout = function() {
    xmlhttp.abort();
    alert("Данные не получены от сервера (Timeout request).");
  }
  xmlhttp.open('POST', '/action', true); // Открываем асинхронное соединение
  xmlhttp.setRequestHeader('Cache-Control','max-age=3600');
  xmlhttp.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded');  
  if(mAction.length)
    xmlhttp.send("resourcebyhost=auditlist&resourcehost=" + mHost + "&resourcepath=" + mAction); 
  else
    xmlhttp.send("resourcebyhost=auditlist&resourcehost=" + mHost);   
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
          alert(xmlhttp.responseText);
		  var NamesData   = ListOfNames.data;
          for(i in ListOfNames.data) {                                      
             iDivObj += 1;
          } 
          $("#tableHostPrint").html("<div id=\"audit_block\" class=\"dir_form_select\"></div>");                        
          if(iDivObj == 0) pathObj = "..";
;                    
          $('#tableHostPrint').load('html/main_audit_tmp_files.html', '.main_audit_div_folder', function(){  
            LoadTemplateContentAuditFilesMain(mHost, mAction, ListOfNames, mConfig);
          });  
          
           $('#tableHostPrint').html(out);            
            if(mConfig != ""){                    
              $('.auditblock_labels').load('html/audit_tmp_rules.html', '.rules_audit_div', function(){  
                var block  =  $(".auditblock_labels").html();  
                if(iDivObj > 0)
                  LoadTemplateContentAuditList(mHost, mAction, ListOfNames.data[i].rules,block);
                else
                  LoadTemplateContentAuditList(mHost, mAction, "", block);
              });                                             
            }
            else{
              $('.auditblock_labels').load('html/audit_tmp_files.html', '.rules_audit_div_folder', function(){  
                LoadTemplateContentAuditFiles(mHost, mAction);
              });  
            }
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
  xmlhttp.timeout = 30000;
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
              $("#tableHostPrint").html("<img class=\"system_image\" id=\"file_go\"><h3>Обновление списка аудита...</h3>")
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


function AuditJournalAction(mHost, mAction) {
 // alert('hostResources!');
  $('.jnlList').html("");
  $(".jnl").html("<img class=\"system_image\" id=\"file_go\"><h3>Загрузка данных журнала аудита...</h3>");
  document.getElementById('table_save_audit_folder').style.display = "none";
 
  var xmlhttp = getXmlHttp(); // Создаём объект XMLHTTP
  xmlhttp.timeout = 30000;
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
            $('.auditblock_labels').append("<div class=\"jnlList\"><div class=\"jnl\"><h3>Журнал аудита ресурса:<\h3><span><p>"  + mAction + "</p></span></div></div>").ready(function () {
            for(i in ListOfNames.data) { 
              $('.auditblock_labels .jnl').append("<span><p>"  + ListOfNames.data[i] + "</p></span>");    
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




function ResourceAttribute(mHost, mAction) {
  //alert('AttributeResources!');
   $("#tableHostPrint").html("<img class=\"system_image\" id=\"file_go\"><h3>Обновление данных атрибутов...</h3>")
  var xmlhttp = getXmlHttp(); // Создаём объект XMLHTTP
  xmlhttp.timeout = 30000;
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
                   
          out += "<span id=\"glowtext\" onclick=\"CheckLoader(this);hostAction(\'" + mHost + "\',\'" + mAction.substring(0, mAction.lastIndexOf('/')) + "/\');\"><img class=\"system_image\">Вернуться к списку файлов и директорий</span><br>";          
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
              AclObjects += HTMLACLRules(mHost, mAction,type_acl, type_acl_par,ListOfNames.data.acl_attributes.acl_node[i]);
              AclCnt++;
           }    
            //default rules for OWNER USER GROUP
           if(ListOfNames.data.acl_attributes.acl_node[i].res_what == "" && 
                ListOfNames.data.acl_attributes.acl_node[i].default != undefined &&
                (type_acl_par == "u" || type_acl_par == "g" || type_acl_par == "o")){
                var defContent = "";                
                defContent  += HTMLDefineBlock(type_acl_par, ListOfNames.data.acl_attributes.acl_node[i].default);
                if(type_acl_par == "u"){
                  DefAclObjectUser      = defContent;;
                }
                else if(type_acl_par == "g"){
                  DefAclObjectGroup      = defContent;  
                }
                else if(type_acl_par == "o"){
                  DefAclObjectOther      = defContent;
                }
                                    
            }
            iDivObj += 1;
          } 
          $('#tableHostPrint').html("");
          if(iDivObj > 0){  
            var rec_check = (g_checked_rec == true)?"checked":"";
            out += "<div id=\"main_rights_block\"><span><p id=\"controls_rec\"><input type=\"checkbox\" id=\"s_recursive\" value=\"recursive\" " + rec_check + "> Применить рекурсию</p></span>";
            //Discrete Bits 
            out += DiscreteBitsHtmlBlock(NamesOwner, NamesGroup, NamesFlags);
            var PermAll = "";
            PermAll += PermOwner + PermGroup + PermOther;
            //Discrete Rules
            out += DiscreteHtmlBlock(mHost, mAction, PermAll, DefAclObjectUser, DefAclObjectGroup, DefAclObjectOther);
            //ACL Rules
            out += ACLHtmlBlock(mHost, mAction, AclObjects); 
            //Mandat Rules
            out += "<div id=\"MandatObjects\"></div></div>";
            $('#tableHostPrint').html(out);   
            $('#ResourceCurrent').html(mHost + "  " +  NamesFile);  
            ResourceLocalObjects(mHost,"Users");  
            ResourceMandatObjects(mHost, mAction); 
            UpdateRulesEditForm ();
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



function ResourceDiscrete(mHost, mAction, mValueOld, mValue, mRec, mBits) {
  //alert('AttributeDiscrete!');
  var xmlhttp = getXmlHttp(); // Создаём объект XMLHTTP
  xmlhttp.timeout = 30000;
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
      var bValue = (document.getElementById("s_bit_suid").checked)?"1":"0";  
      HttpDataParams += "&s_bit_suid=" + bValue;
    }
    if(mBits&(1<<1)){
      var bValue = (document.getElementById("s_bit_sguid").checked)?"1":"0";  
      HttpDataParams += "&s_bit_sguid=" + bValue;
    }
    if(mBits&(1<<2)){
      var bValue = (document.getElementById("s_bit_sticky").checked)?"1":"0";  
      HttpDataParams += "&s_bit_sticky=" + bValue;
    }      
  }
  xmlhttp.send(HttpDataParams);       
  xmlhttp.onreadystatechange = function() { // Ждём ответа от сервера
    if (xmlhttp.readyState == 4) { // Ответ пришёл        
      if(xmlhttp.status == 200) { // Сервер вернул код 200 (что хорошо)                    
          //alert(xmlhttp.responseText);  
          var ListOfNames = JSON.parse(xmlhttp.responseText);
		  var NamesData   = ListOfNames.data;
          if(ListOfNames.data.update == "OK"){
            ResourceAttribute(mHost, mAction); 
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


function ResourceLocalObjects(mHost, mType) {
  //alert('Attribute!');
  var xmlhttp = getXmlHttp(); // Создаём объект XMLHTTP
  xmlhttp.timeout = 30000;
  xmlhttp.ontimeout = function() {
    xmlhttp.abort();
    alert("Данные не получены от сервера (Timeout request).");
  }
  xmlhttp.open('POST', '/action', true); // Открываем асинхронное соединение
  xmlhttp.setRequestHeader('Cache-Control','max-age=3600');
  xmlhttp.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded');  
  var HttpDataParams = "";
  if(mType == "Users")
    HttpDataParams += "resourcebyhost=localuserlist&resourcehost=" + mHost;   
  else if(mType == "Groups")
    HttpDataParams += "resourcebyhost=localgroupslist&resourcehost=" + mHost;
  xmlhttp.send(HttpDataParams);       
  xmlhttp.onreadystatechange = function() { // Ждём ответа от сервера
    if (xmlhttp.readyState == 4) { // Ответ пришёл        
      if(xmlhttp.status == 200) { // Сервер вернул код 200 (что хорошо)                    
          //alert(xmlhttp.responseText);  
          var ListOfNames = JSON.parse(xmlhttp.responseText);		  
          var out_list ="<select id=\"AclObjectsWho\">";
          for(i in ListOfNames.data) {  
            var NamesData   = ListOfNames.data[i];            
            out_list += "<option value=\'" + NamesData + "\'>" + NamesData + "</option>";
         }
         out_list +="</select>";
         $('#ListObjects').html("");
         $('#ListObjects').html(out_list);

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


function ResourceLocalObjectsSelect(mHost, mType, mSelect, mDomain) {
  var HttpDataParams = "resourcehost=" + mHost;
  var xmlhttp = getXmlHttp(); // Создаём объект XMLHTTP
  xmlhttp.timeout = 30000;
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
            $(mSelect).append("<option value=\"" + NamesData +"\">"+ NamesData +"</option>");                        
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



function ResourceMandatObjects(mHost, mAction) {
  //alert('AttributeMandat!');
  $("#MandatObjects").html("<img class=\"system_image\" id=\"file_go\"><h3>Обновление атрибутов контроля целостности....</h3>");
  var xmlhttp = getXmlHttp(); // Создаём объект XMLHTTP
  xmlhttp.timeout = 30000;
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
          var out_list  = "";
          out_list += MandatHtmlBlock(mHost, mAction, ListOfNames);                   
          $('#MandatObjects').html("");
          $('#MandatObjects').html(out_list);
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
  xmlhttp.timeout = 30000;
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
  xmlhttp.timeout = 30000;
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
            ResourceAttribute(mHost, mAction); 
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
  xmlhttp.timeout = 30000;
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
              var selectWho = document.getElementById("AclObjectsWho"); 
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
            ResourceAttribute(mHost, mAction); 
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

function HTMLACLRules(mHost, mAction, type_acl,type_acl_par, mListAlsObj){
  var AclObj = "";
  AclObj += "<tr><td>" + type_acl + "</td>";
  var mWhat = mListAlsObj.res_what; 
  var type_acl = type_acl_par +  ":" + mWhat;
  if(mWhat == "")
      mWhat = "-";
  AclObj += "<td>" + mWhat + "</td>";
  AclObj += "<td>" + mListAlsObj.res_mask + "</td>";                
  AclObj+= "<td><span id=\"glowtext\" onclick=\"ResourceDeleteAcl(\'" + mHost + "\',\'" + mAction + "\',\'" + type_acl + "\',\'0\');\"><img class=\"system_image\" src=\"images/delete_icon.png\"></span></td>";            
  if(mListAlsObj.default != undefined){                                  
    var defObj  = "default";                        
    defObj  += ":" + mListAlsObj.default.res_obj;
    defObj  += ":" + mListAlsObj.default.res_what;
    AclObj  += "<td>" + mListAlsObj.default.res_mask + "</td>";
    AclObj  += "<td><span id=\"glowtext\" onclick=\"ResourceDeleteAcl(\'" + mHost + "\',\'" + mAction + "\',\'" + defObj + "\',\'0\');\"><img class=\"system_image\" src=\"images/delete_icon.png\"></span></td>";
  }
  else
    AclObj += "<td></td><td></td>";
    AclObj += "</tr>";
   return AclObj;
}

function  HTMLDefineBlock(type_acl_par, ListDefaults) {
    var defObj  = "default";      
    var defContent = "";               
    var bit_r = (ListDefaults.res_mask[0] == "r")?"checked":"-";
    var bit_w = (ListDefaults.res_mask[1] == "w")?"checked":"-";
    var bit_e = (ListDefaults.res_mask[2] == "x")?"checked":"-";            
    defObj      += ":" + ListDefaults.res_obj;                
    defContent  += "<div><input type=\"checkbox\" id=\"" + type_acl_par + "_default_bit_r\" value=\"own_bit_r\" "  + bit_r + "><img class=\"read_state\"><input type=\"checkbox\" id=\"" + type_acl_par + "_default_bit_w\" value=\"own_bit_w\" "  + bit_w + "><img class=\"write_state\"><input type=\"checkbox\" id=\"" + type_acl_par + "_default_bit_e\" value=\"own_bit_e\" "  + bit_e + "><img class=\"run_state\"></div>";             
    return defContent;                   
}
                    
function DiscreteBitsHtmlBlock(NamesOwner, NamesGroup, NamesFlags){                    
  var BitsBlock  = "";
  var bit_suid   = "";
  var bit_sguid  = "";
  var bit_sticky = "";
                          
  if(NamesFlags[0] == "s") bit_suid = "checked";
  if(NamesFlags[1] == "s") bit_sguid = "checked";
  if(NamesFlags[2] == "t") bit_sticky = "checked";
                          
  BitsBlock += "<table id=\"pathtable\"><tr><th colspan=\"4\">Дискреционные атрибуты</th></tr><tr><td colspan=\"4\"><input type=\"checkbox\" id=\"s_bit_suid\" value=\"" + bit_suid +"\" " + bit_suid + "> SUID<br>\
  <input type=\"checkbox\" id=\"s_bit_sguid\" value=\"" + bit_sguid + "\" " + bit_sguid + "> SGUID<br>\
  <input type=\"checkbox\" id=\"s_bit_sticky\" value=\"" + bit_sticky + "\" " + bit_sticky + "> Sticky бит</td></tr><tr><th>Владелец</th><th>Группа</th><th></th><th></th></tr>";
  BitsBlock += "<tr><td>" + NamesOwner + "</td><td>" + NamesGroup +"</td><td></td><td></td></tr>";                                      
  return BitsBlock;           
}            

function DiscreteHtmlBlock(mHost, mAction, PermAll, DefAclObjectUser, DefAclObjectGroup, DefAclObjectOther){
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
  
  DiscreteBlock += "<tr><th></th><th>Атрибуты</th><th>По умолчанию</th><th></th></tr><tr><td>Права пользователя</td><td>\
  <div><input type=\"checkbox\" id=\"discrete_bit_8\" value=\"own_bit_r\" "  + bit_owner_r + ">\
  <img class=\"read_state\">\
  <input type=\"checkbox\" id=\"discrete_bit_7\" value=\"own_bit_w\" "  + bit_owner_w + ">\
  <img class=\"write_state\">\
  <input type=\"checkbox\" id=\"discrete_bit_6\" value=\"own_bit_e\" "  + bit_owner_e + ">\
  <img class=\"run_state\"></div></td><td>" + DefAclObjectUser + "</td><td></td></tr>\
  <tr><td>Права группы</td>\
  <td><div><input type=\"checkbox\" id=\"discrete_bit_5\" value=\"group_bit_r\" " + bit_group_r + ">\
  <img class=\"read_state\">\
  <input type=\"checkbox\" id=\"discrete_bit_4\" value=\"group_bit_w\" " + bit_group_w + ">\
  <img class=\"write_state\">\
  <input type=\"checkbox\" id=\"discrete_bit_3\" value=\"group_bit_e\" " + bit_group_e + ">\
  <img class=\"run_state\"></div></td><td>" + DefAclObjectGroup + "</td><td></td></tr>\
  <tr><td>Права остальных</td>\
  <td><div><input type=\"checkbox\" id=\"discrete_bit_2\" value=\"other_bit_r\" " + bit_other_r + ">\
  <img class=\"read_state\">\
  <input type=\"checkbox\" id=\"discrete_bit_1\" value=\"other_bit_w\" " + bit_other_w + ">\
  <img class=\"write_state\">\
  <input type=\"checkbox\" id=\"discrete_bit_0\" value=\"other_bit_e\" " + bit_other_e + ">\
  <img class=\"run_state\"></div></td><td>" + DefAclObjectOther + "</td><td></td></tr></table><button class=\"defbutton\" name=\"b_discrete_save\" onclick=\"take_discrete_chmod(\'" + mHost + "\', \'" + mAction + "\',\'" + octVal + "\');\"><img class=\"system_image\" src=\"images/task-complete.png\"> Сохранить</button>";  
  return DiscreteBlock; 
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
    var EditMandatLabels = MandatHtmlAddBlock(mHost, mAction, mBlock, MaskFlags);
    
    MBlock += "<table id=\"pathtable\"><tr><th colspan=\"4\">Мандатные атрибуты файла</th></tr>\
    <tr>\
		<td>Уровень конфиденциальности</td>\
		<td>Целостность</td>\
		<td>Категории</td>\
		<td>Атрибуты</td></tr>\
		<td>" + LevelMand  + "</td>\
		<td>" + LevelMandC + "</td>\
		<td>" + CategoriesLevel  + "</td>\
		<td>" + mandat_bit_labal + "</td></tr>\
        </table>" + EditMandatLabels + "<button class=\"defbutton\" name=\"b_mdt_in\" onclick=\"showInfoDev('table_save_mandat')\" ><img class=\"system_image\" src=\"images/settings-icon.png\"> Редактировать мандатные ПРД</button>";
    return MBlock;
}

function ACLHtmlBlock(mHost, mAction, AclObjects){
    var ACLBlock = "";
    var ACLBlockAdd = "<div id=\"table_save_acl\" class=\"div_menu\"><table class=\"leftcoltable\"><tr><th colspan=\"2\">Добавление правила ACL</th></tr>\
    <tr><td>Тип субъекта ACL</td><td><span><input name=\"dradio_type\" onclick=\" ResourceLocalObjects(\'" + mHost + "\',\'Users\')\" type=\"radio\" value=\"u:\" checked> Пользователь</span><span><input name=\"dradio_type\" onclick=\" ResourceLocalObjects(\'" + mHost + "\',\'Groups\')\" type=\"radio\" value=\"g:\"> Группа</span><span><input name=\"dradio_type\" type=\"radio\" value=\"m:\"> Маска</span>\
    </td></tr>\
    <tr><td>Имя</td>\
    <td><div id=\"ListObjects\"></div>\
    </td></tr>\
    <tr><td>Атрибуты</td>\
    <td><div><span><input type=\"checkbox\" id=\"acl_bit_r\" value=\"acl_bit_r\"><img class=\"read_state\">\
    <input type=\"checkbox\" id=\"acl_bit_w\" value=\"acl_bit_w\"><img class=\"write_state\">\
    <input type=\"checkbox\" id=\"acl_bit_e\" value=\"acl_bit_e\"><img class=\"run_state\"></span></div>\
    </td></tr>\
    <tr><td>По умолчанию</td>\
    <td><span><input type=\"checkbox\" id=\"acl_def_bit\" value=\"default:\">  По умолчанию</span>\
    </td></tr></table>\
    <br><span><button class=\"defbutton\" name=\"b_acl_in\"  onclick=\"ResourceAddAcl(\'" + mHost + "\',\'" + mAction + "\')\"><img class=\"system_image\" src=\"images/task-complete.png\"> Сохранить</button>  <button class=\"defbutton\" name=\"b_acl_cancel\" onclick=\"showInfoDev('table_save_acl');\" ><img class=\"system_image\" src=\"images/delete.png\"> Отмена</button></span></div>";    
    ACLBlock +="<table id=\"pathtable\"><tr><th colspan=\"6\">Субъекты ACL</th></tr><tr><td>Тип субъекта ACL</td><td>Имя</td><td>Правило</td><td></td><td>По умолчанию</td><td></td></tr>" + AclObjects + "</table>" + ACLBlockAdd + "<button class=\"defbutton\" name=\"b_acl_add\" onclick=\"showInfoDev('table_save_acl');\" ><img class=\"system_image\" src=\"images/settings-icon.png\"> Добавить правило ACL</button>\
    <button class=\"defbutton\" name=\"b_acl_delete_all\" onclick=\"ResourceDeleteAcl(\'" + mHost + "\',\'" + mAction + "\',\'\',\'1\')\"><img class=\"system_image\" src=\"images/delete.png\"> Удалить все правила</button>";
               /*Add acl rules*/            
    return ACLBlock;
}



function MandatHtmlAddBlock(mHost, mAction, MandatObjects, MaskFlags){
    var MandatBlock = "";
    /**/
    var out_list0 = "";
    out_list0 += "<select id=\"MandatObjectLevals\">";
    for(i in MandatObjects.data.res_lev) {  
        var CheckLevel = (MandatObjects.data.mandat_rules.res_mandat_1 == i)?"selected":"";       
        out_list0 += "<option value=\'" + MandatObjects.data.res_lev[i].res_lev_id + "\' " + CheckLevel + ">" + MandatObjects.data.res_lev[i].res_lev_name + "</option>";

    }
    out_list0 +="</select>";
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
    var out_list2 = "";
    out_list2 += "<select id=\"MandatObjectMonolite\">";
    for(var q = 0; q < 64; q++) {  
      var NonoliteLevel = (MandatObjects.data.mandat_rules.res_mandat_2 == q)?"selected":"";
      out_list2 += "<option value=\'" + i + "\' " + NonoliteLevel + ">" + q + "</option>";
    }
    var AttrFlags  = hexToDec(MandatObjects.data.mandat_rules.res_mandat_4);
    var flag_ccnr  = (AttrFlags&(1<<0))?"checked":"";
    var flag_ccnri = (AttrFlags&(1<<1))?"checked":"";
    var flag_ehole = (AttrFlags&(1<<2))?"checked":"";
    var flag_whole = (AttrFlags&(1<<3))?"checked":"";
    out_list2 +="</select>";
    MandatBlock += "<div id=\"table_save_mandat\" class=\"div_menu\">\
    <table class=\"leftcoltable\" >\
    <tr><th colspan=\"2\">Редактирование мандатных ПРД</th></tr>\
    <tr><td>Уровень конфиденциальности</td><td>" + out_list0 + "</td></tr>\
    <tr><td>Целостность</td><td>" + out_list2 + "</td></tr>\
    <tr><td>Разряд/Категория</td><td>" + out_list1 + "</td></tr>\
    <tr><td>Атрибуты</td><td><input type=\"checkbox\" id=\"flag_ccnr\"  value=\"ccnr\"" + flag_ccnr + "> ccnr<br>\
    <input type=\"checkbox\" id=\"flag_ccnri\" value=\"ccnri\"" + flag_ccnri + "> ccnri<br>\
    <input type=\"checkbox\" id=\"flag_ehole\" value=\"ehole\"" + flag_ehole + "> ehole<br>\
    <input type=\"checkbox\" id=\"flag_whole\" value=\"whole\"" + flag_whole + "> whole<br>\
    </td></tr></table><button class=\"defbutton\" name=\"b_mandat_in\" onclick=\"ResourceMandatObjectsSave(\'" + mHost + "\', \'" + mAction + "\');showInfoDev('table_save_mandat');\"><img class=\"system_image\" src=\"images/task-complete.png\"> Сохранить</button>  <button  class=\"defbutton\" name=\"b_mandat_cancel\" onclick=\"showInfoDev('table_save_mandat')\" ><img class=\"system_image\" src=\"images/delete.png\"> Отмена</button></div>";  
    return MandatBlock;
}


function take_discrete_chmod(mHost, mAction, mOld){
 var s_bit_new   = 0;
 var s_bit_suid    =  (document.getElementById("s_bit_suid").value == "checked")?true:false;
 var s_bit_sguid   =  (document.getElementById("s_bit_sguid").value == "checked")?true:false;
 var s_bit_sticky  =  (document.getElementById("s_bit_sticky").value == "checked")?true:false;
 
 var s_bit_suid_new    =  (document.getElementById("s_bit_suid").checked)?true:false;
 var s_bit_sguid_new   =  (document.getElementById("s_bit_sguid").checked)?true:false;
 var s_bit_sticky_new  =  (document.getElementById("s_bit_sticky").checked)?true:false;
 
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
     (s_bit_new != 0)){
      /*update Discrete*/
    var rec = "";
  if ($('#s_recursive').is(':checked')) 
    rec = "-R";   
    //alert(octValNew);
    ResourceDiscrete(mHost, mAction, mOld, octValNew, rec, s_bit_new);
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
    if(element == 'table_save_audit_folder') $('.jnlList').html("");
}


function decToHex(n){ return Number(n).toString(16); }

function hexToDec (hex){ return parseInt(hex,16); }


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
      ResourceLocalObjectsSelect(host, mTypeSub, '#audit_rule_who_id_', mDomain);
   });
    
    $('#add_audit_rule_submit_id_').click(function() {  
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
      showInfoDev('table_save_audit');
            
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
      showInfoDev("table_save_audit_folder");
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
  $('.device_tmp_cols_1').text(function(index, oldText){                                 
    oldText = (index + 1).toString();
    return oldText;
  });
  $('.device_tmp_cols_2').text(function(index, oldText){                             
    oldText    =  ListOfNames.data[index].host_hame;
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
          oldText += "<span>Изменено: " + tmp[0]  + "</span><br>"
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
        iTimeObj = tmp[0];
      }            
    }                 
    oldText    =  iTimeObj;
    return oldText;
  });
  $('.device_tmp_cols_6').html(function(index, oldText){                                                 
    oldText    =  ((ListOfNames.data[index].host_alive == "OK")?"<span><font color=\"green\">устройство готово к работе</font></span>":"<span><font color=\"red\">устройство недоступно</font></span>");
    return oldText;
  });
  $('.device_tmp_cols_7').html(function(index, oldText){                             
    oldText    = "<span id=\"glowtext\" onclick=\"hostAction('" + ListOfNames.data[index].host_hame + "','/');\"><img class=\"system_image\" src=\"images/settings-icon.png\" onclick=\"Check(this);\"></span>";
  return oldText;
  });;
  $('.device_tmp_cols_8').html(function(index, oldText){ 
    oldText    = "<span id=\"glowtext\" onclick=\"AuditAction('" + ListOfNames.data[index].host_hame + "','/','');\"><img class=\"system_image\" src=\"images/settings-icon.png\" onclick=\"Check(this);\"></span>";
    return oldText;
  });        
}

function LoadTemplateContentDir(mHost, url, ListOfNames){
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
          oldText = "<span id=\"glowtext\" onclick=\"DirAction(\'" + mHost + "\',\'" + ListOfNames.data[index - 1].res_base + "/\');\"><img class=\"system_image\" id=\"folder_img_gr\"> " + pathObj + "</span>";
        }
      }
      else{                     
        pathObj = ListOfNames.data[index - 1].res_hame;
        oldText = '<span id=\"glowtext\" onclick=\"take_path(\'' + mHost + '\' , \''+ ListOfNames.data[index - 1].res_base + '\');\"><img class=\"system_image\" id=\"file_img_gr\">' + pathObj + '</span>';
      }
    }
    else
      oldText = "<span id=\"glowtext\"  onclick=\"DirAction(\'" + mHost + "\',\'"+ url + "/\');\"><img class=\"system_image\" id=\"folder_img_gr\"></span>";
      return oldText;
    });     
}



function LoadTemplateFilesDevuce(mHost, url, ListOfNames){
  $(".left_file_box_icon").html(function(index, oldText){ 
    if(index > 0){
      oldText = "<span id=\"glowtext\" onclick=\"ResourceAttribute(\'" + mHost + "\',\'" + ListOfNames.data[index - 1].res_base + "\');\"><img class=\"system_image\" src=\"images/settings-icon.png\"></span>";
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
                ListOfNames.data[index - 1].res_base + "/\');\"><img class=\"system_image\" id=\"folder_img_gr\"> " + pathObj + "</span>";
        }
      }
      else{                     
        pathObj = ListOfNames.data[index - 1].res_hame;
        oldText = '<span id=\"glowtext\" onclick=\"take_path(\'' + mHost + '\' , \''+ ListOfNames.data[index - 1].res_base + '\');\"><img class=\"system_image\" id=\"file_img_gr\">' + pathObj + '</span>';
      }
    }
    else
      oldText = "<span id=\"glowtext\"  onclick=\"hostAction(\'" + mHost + "\',\'"+ url + "/\');\"><img  class=\"system_image\" id=\"folder_img_gr\"></span>";
    
      return oldText;
  }); 
  /*
  $('.system_image').click(function() {  
      $(this).prop('src', 'images/loader.gif');
    });  
 */
}
 



