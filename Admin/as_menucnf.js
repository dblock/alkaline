document.writeln('<script type="text/javascript" src="xparse.js"></script>');

function ASBuildConfigurations(Menu) {
  var ConfigXMLString = new String("$configxml");
  var XParsedObject = new Array();
  var ConfigurationsAliases = new String("");
  var ArrayOfConfigurations = new Array();

  XParsedObject = Xparse(ConfigXMLString);
 
  ConfigurationsAliases = XParseGetValue("/configurations/head/list", XParsedObject);
  ArrayOfConfigurations = ConfigurationsAliases.split(";");
 
  for (var i=0; i < ArrayOfConfigurations.length; i++) {        	
    Menu.MTMAddItem(new MTMenuItem(ArrayOfConfigurations[i],
        "as_config.html?name=" + escape(ArrayOfConfigurations[i]), 
        "text", "alkaline_config.gif"));
  }
}

fAsMenuCnfLoaded = true;

// Alkaline Search Engine
// (c) Vestris Inc. - 2001 All Rights Reserved
// http://alkaline.vestris.com/
