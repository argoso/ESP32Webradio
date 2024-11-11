const char MAIN_page[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
  <style>
  body {
    text-align:center;
    font-family: helvetica;
  }
  </style>
  <head>
    <title>
      Webradio
    </title>
  <head>
  <body>
    <h1>Veebiraadio WiFi seadistus</h1>
    *mark1begin*
    <h3>WiFi seaded</h3>
    <form method="POST">
      <p>
        <h4>WLAN-Name (SSID):</h4>
        <input type="text" name="ssid" value="*ssid*">
      </p>
      <p>
        <h4>WLAN-Passwort:</h4>
        <input type="password" name="password" value="*password*">
      </p>
      <!--<p>
        <h4>Time server (NTP):</h4>
        If it is not clear what this is, <br/>then it should <b>at.pool.ntp.org</b> remain.<br/>
        <input type="text" name="ntpserver" value="*ntpserver*">
      </p>-->
      <input type="submit" value="submit">
    </form>
    *mark1end*
    <p>
      *feedback*
    </p>
  </body>
</html>
)=====";
