// Добавляет очередной каталог 
function showHFWindow(host, func) {
  var isLastTypeDir = true;
    $("#hf_insert_path").text("");
    $("#hf_bottom_panel button").off('click');
    $("#hf_window img").off('click');
    $("#hf_window").dialog({
      width: 'auto',
      height: 'auto'
    });
    let tbControlFS = $("#hf_tbl_filesystem");
    requestDirList("/");
    // Задаем последнее имя
    function setCurrentName(obj, isDirectory) {
      var name = $(obj).text();
      isLastTypeDir = isDirectory;
      $("#hf_insert_path").text(name);
    }
    /* Открыть директорию */
    function openDir(obj) {
      var dirName = $(obj).text();
      var fsPathName = $("#hf_fs_path_name").text();
      requestDirList($("#hf_fs_path_name").text() + (fsPathName === "/" ? "" : "/") + dirName);
      $("#hf_insert_path").text("");
    }
    /* Запрос списка объектов внутри каталога */
    function requestDirList(path)
    {
      tbControlFS.empty();
      $("#hf_fs_path_name").text(path)
      sendAjaxRequest("/antivirus", "get=dir_list&host=" + host + "&dir=" + path, function(data) {
        var listOfNames = JSON.parse(data);
        for (var i in listOfNames.data) {
          addRow(listOfNames.data[i].fs_name, listOfNames.data[i].fs_type);
        }
      });
      /* Добавление строки в таблицу */
      function addRow(fs_name, fs_type) {
        var trObj = $('<tr>');
        if(fs_type === "file") {
          createFSImageTD("images/state_write.png");
          createFileTD(fs_name);
        }
        else if(fs_type === "dir") {
          createFSImageTD("images/Folder-icon.png");
          createDirTD(fs_name);
        }
        tbControlFS.append(trObj);
        /* Создание элемента td с файлом в строке таблицы */
        function createFileTD(text) {
          $('<td/>', {align: "left", text: text, on: { click: function(event){setCurrentName(this, false);}},}).appendTo(trObj);
        }
        /* Создание элемента td с директорией в строке таблицы */
        function createDirTD(text) {
          $('<td/>', {align: "left", text: text, on: {
            click: function(event){setCurrentName(this, true);},
            dblclick: function(event){openDir(this);}},}).appendTo(trObj);
        }
        /* Создание элемента td с картинкой (файл/папка) в строке таблицы */
        function createFSImageTD(img_name) {
          $('<td/>', {align: "center", append: $('<img>', { src: img_name, width: 24, height: 24 }),}).appendTo(trObj);
        }
      }
    }
    /* Вернуться по дереву каталогов назад */
    $("#hf_window img").click(function () {
      var pathLast = $("#hf_fs_path_name").text();
      var index = pathLast.lastIndexOf("/");
      pathLast = pathLast.substr(0, index);
      if(0 === pathLast.length) {
        pathLast = "/";
      }
      requestDirList(pathLast);
    });
    // Отработка кнопки Ок
    $("#hf_bottom_panel button").click(function() {
      $('#hf_window').dialog("close");
      var name = $("#hf_fs_path_name").text();
      var hfInsertPath = $("#hf_insert_path").text();
      var lastPath = $("#hf_insert_path").text() === "" ? "" : "/" + $("#hf_insert_path").text();
      name = (name === "/" ? "" : name) + lastPath;
      if(name === '' && $("#hf_insert_path").text() === "") {
        name = "/";
      }
      func(name, isLastTypeDir);   // Функция выдачи
    });
}