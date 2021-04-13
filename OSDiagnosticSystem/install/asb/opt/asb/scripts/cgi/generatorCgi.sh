if [[ ! -e /usr/lib/cgi-bin ]]; then sudo mkdir -p /usr/lib/cgi-bin; fi;
if [[ ! -e /usr/lib/cgi-bin/generatorCgi ]]; then  echo "Не найден файл generatorCgi в каталоге /usr/lib/cgi-bin. Для натсройки требуется установка генератора паролей."; 
else 
  echo "Файл  generatorCgi проверен.Старт настройки...";
if [[ ! -e /etc/apache2/sites-available ]]; then echo "Apache2 dir sites-available not found..."; 
else 
 sudo cp ./000-default.conf /etc/apache2/sites-available/; sudo chmod 0777 /etc/apache2/sites-available/000-default.conf;
 sudo cp ./000-default.conf /etc/apache2/sites-enabled/; sudo chmod 0777 /etc/apache2/sites-enabled/000-default.conf;
 
 if [[ "$(sudo cat /etc/apache2/apache2.conf | grep "AstraMode off")" == "" ]]; then echo "Apache2 incorrect apache2.conf ... repair"; echo "AstraMode off" >> /etc/apache2/apache2.conf; else echo  "Apache2 apache2.conf ... test"; fi;
 
 if [ "$(sudo systemctl status apache2.service  2>&1 | sudo grep "active (running)")" ]; then echo "apache2.service  active";   else echo "apache2.service  not active... start..."; sudo systemctl start  apache2.service ;  fi
 
 if [[ ! -e /etc/apache2/mods-enabled ]]; then echo "Apache2 dir mods-enabled not found..."; 
 else
   if [[ ! -e /etc/apache2/sites-available/000-default.conf  ||  ! -e /etc/apache2/sites-enabled/000-default.conf ]]; then echo "Apache2 000-default.conf copy error...";
     else
      echo "Apache2 dir mods-enabled sucess...";
      cd /etc/apache2/mods-enabled;
      sudo ln -s ../mods-available/cgi.load;
      sudo a2enmod headers;
      sudo service apache2 reload;
      echo "Apache2: настройка выполнена.";
   fi;
 fi;  
fi;
fi;

