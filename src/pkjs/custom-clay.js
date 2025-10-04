module.exports = function (minified) {
  var clayConfig = this;
  var _ = minified._;
  var $ = minified.$;

  // Define presets
  var presets = {
    default: {
      BackgroundColor1: "FFFFFF",
      BackgroundColor2: "AAAAAA",
      TextColor1: "FFFFFF",
      TextColor2: "000000",
      TextColor3: "000000",
      DateColor: "000000",
      HoursHandColor: "FFFFFF",
      HoursHandBorderColor: "000000",
      MinutesHandColor: "FFFFFF",
      MinutesHandBorderColor: "000000",
      SecondsHandColor: "FF5500",
      },
    orangeDreams: {

    },
    terminalGreen: {

    },
    mauveTheme: {

    },
    fireworkTheme: {

    },
    lightOceanTheme: {

    },
    roseTheme: {

    },
    oceanTheme: {

    },
    sandTheme: {

    },
    greyTheme: {

    },
    userTeal1: {

    },
    bwTheme1: { //White background
      BWDateColor:"000000", //GColorBlack;
      BWBackgroundColor1:"FFFFFF", //GColorWhite;
      BWBackgroundColor2:"555555", //GColorDarkGray;
      BWTextColor1:"FFFFFF",  //GColorWhite;
      BWTextColor2: "000000", //GColorBlack;
      BWBTQTColor: "000000"
    },
    bwTheme2: { //inverted, black background
      BWDateColor:"FFFFFF",
      BWBackgroundColor1:"000000",
      BWBackgroundColor2:"555555",
      BWTextColor1:"000000",
      BWTextColor2: "FFFFFF",
      BWBTQTColor: "000000"
    }
  };

  function decimalToHex(decimalColor) {
    return ('0' + decimalColor.toString(16)).slice(-2);
  }

  function convertToHexFromDecimal(decimal) {
    var r = (decimal >> 16) & 0xFF;
    var g = (decimal >> 8) & 0xFF;
    var b = decimal & 0xFF;
    return '#' + decimalToHex(r) + decimalToHex(g) + decimalToHex(b);
  }

  function toggleCustomSection(show) {
    var customFields = [
      'BackgroundColor1',
      'BackgroundColor2',
      'TextColor1',
      'TextColor2',
      'TextColor3',
      'DateColor',
      'HoursHandColor',
      'HoursHandBorderColor',
      'MinutesHandColor',
      'MinutesHandBorderColor',
      'SecondsHandColor'

    ];

    var items = [];
    for (var i = 0; i < customFields.length; i++) {
      var key = customFields[i];
      var item = clayConfig.getItemByMessageKey(key);
      items.push(item);
    }

    for (var j = 0; j < items.length; j++) {
      var currentItem = items[j];
      if (currentItem) {
        if (show) {
          currentItem.show();
        } else {
          currentItem.hide();
        }
      }
    }
  }



  function applyPreset() {
    var presetSelector = clayConfig.getItemByMessageKey('SETTING_PRESET');
    var selectedPreset = presetSelector.get();

    if (selectedPreset === "custom") {
      toggleCustomSection(true);
    } else {
      toggleCustomSection(false);

      var colors = presets[selectedPreset];
      var colorKeys = Object.keys(colors);
      for (var i = 0; i < colorKeys.length; i++) {
        var key = colorKeys[i];
        var item = clayConfig.getItemByMessageKey(key);
        if (item) {
          item.set(colors[key]);
        }
        updateSVGColors(key, colors[key]);
      }
    }
  }


  function attachColorListeners() {
    var colorKeys = Object.keys(presets.default);

    for (var i = 0; i < colorKeys.length; i++) {
      var key = colorKeys[i];

      var colorPicker = clayConfig.getItemByMessageKey(key);
      if (colorPicker) {
        (function (localKey, localColorPicker) {
          colorPicker.on('change', function () {
            var newColor = localColorPicker.get();
            updateSVGColors(localKey, newColor);
          });
        })(key, colorPicker);
      }
    }
  }

  function exportTheme() {
    var colorKeys = Object.keys(presets.default);
    var themeData = {};

    // Convert all colors to hex
    for (var i = 0; i < colorKeys.length; i++) {
      var key = colorKeys[i];
      var colorValue = clayConfig.getItemByMessageKey(key).get();

      // Ensure it's always in #RRGGBB format
      if (typeof colorValue === 'number') {
        themeData[key] = convertToHexFromDecimal(colorValue); // Convert decimal to hex
      } else {
        themeData[key] = '#' + colorValue; // Ensure hex is prefixed with #
      }
    }

    // Convert to JSON
    var jsonData = JSON.stringify(themeData, null, 2);
    var blob = new Blob([jsonData], { type: "application/json" });
    var url = URL.createObjectURL(blob);

    // Create a download link
    var a = document.createElement("a");
    a.href = url;
    a.download = "watchface-theme.json";
    document.body.appendChild(a);
    a.click();
    document.body.removeChild(a);
    URL.revokeObjectURL(url);
  }


  clayConfig.on(clayConfig.EVENTS.AFTER_BUILD, function () {
    var presetSelector = clayConfig.getItemByMessageKey('SETTING_PRESET');
    presetSelector.on('change', applyPreset);

    var isRoundWatch = clayConfig.meta.activeWatchInfo.platform === 'chalk';

    applyPreset();
    attachColorListeners();
    injectPresetSVGs(isRoundWatch);

    var colorKeys = Object.keys(presets.default);
    for (var i = 0; i < colorKeys.length; i++) {
      var key = colorKeys[i];
      var colorValue = clayConfig.getItemByMessageKey(key).get();
      updateSVGColors(key, colorValue);
    }

    // Create export button
    var exportButton = document.createElement("button");
    exportButton.textContent = "Export Theme";
    exportButton.style.cssText = "display: block; margin: 10px auto; padding: 10px; font-size: 16px;";
    exportButton.onclick = exportTheme;

    // Append to settings page
    document.body.appendChild(exportButton);
  });

};
