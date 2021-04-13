if [[ ! -e /usr/lib/cgi-bin ]]; then sudo mkdir -p /usr/lib/cgi-bin; fi;
if [[ ! -e /usr/lib/cgi-bin/generatorCgi ]]; then  echo "Not find file generatorCgi... copy..."; 
sudo cp ./generatorCgi /usr/lib/cgi-bin/; sudo chmod 0777 /usr/lib/cgi-bin/generatorCgi;
else echo "File generatorCgi find...";
fi;
echo "File generatorCgi... start settings...";
if [[ ! -e /etc/apache2/sites-available ]]; then echo "Apache2 dir sites-available not found..."; 
else 
 sudo cp ./000-default.conf /etc/apache2/sites-available/; sudo chmod 0777 /etc/apache2/sites-available/000-default.conf;
 sudo cp ./000-default.conf /etc/apache2/sites-enabled/; sudo chmod 0777 /etc/apache2/sites-enabled/000-default.conf;
 
 if [ "$(sudo systemctl status apache2.service  2>&1 | sudo grep "active (running)")" ]; then echo "apache2.service  active";   else echo "apache2.service  not active... start..."; sudo systemctl start  apache2.service ;  fi
 
 if [[ ! -e /etc/apache2/mods-enabled ]]; then echo "Apache2 dir mods-enabled not found..."; 
 else
   if [[ ! -e /etc/apache2/sites-available/000-default.conf  ||  ! -e /etc/apache2/sites-enabled/000-default.conf ]]; then echo "Apache2 000-default.conf copy error...";
     else
      echo "Apache2 dir mods-enabled sucess...";
      cd /etc/apache2/mods-enabled;
      sudo ln -s ../mods-enabled/cgi.load;
      sudo a2enmod headers;
      sudo  service apache2 reload;
      echo "Apache2 generatorCgi settings complete.";
   fi;
 fi;  
fi;
