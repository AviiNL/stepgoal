module.exports = [
    {
      "type": "heading",
      "defaultValue": "Step Goal Settings"
    },
    {
      "type": "section",
      "items": [
        {
          "type": "input",
          "messageKey": "StepGoal",
          "defaultValue": "10000",
          "label": "Step Goal"
        },
        {
          "type": "toggle",
          "messageKey": "ShowClock",
          "defaultValue": true,
          "label": "Show Clock"
        },
        {
          "type": "toggle",
          "messageKey": "ShowBattery",
          "defaultValue": true,
          "label": "Show Battery"
        },
        {
          "type": "toggle",
          "messageKey": "ShortNotation",
          "defaultValue": false,
          "label": "Short Step Notation"
        }
      ]
    },
    {
      "type": "submit",
      "defaultValue": "Save Settings"
    }
  ];