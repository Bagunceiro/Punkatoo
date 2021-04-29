function addEntry(ele, net, conf) {
  ele +=
    "<tr><td>" +
    "<input type=checkbox checked id=" +
    "cf0" +
    " name=conf /><label for=" +
    "cf0" +
    ">&nbsp;</label>" +
    "<input type=hidden name=ssid value=" +
    net.ssid +
    "/></td>" +
    "<td>";
  if (conf) ele += "<a href=/config.netedit?ssid=" + net.ssid + ">"; // Only confs
  if (net.open) ele += "🔓";
  else ele += "🔒";
  ele += net.ssid;
  if (conf) ele += "</a>"; // only confs
  ele += "</td></tr>";
}

function netdata(obj) {
  for (const [key, value] of Object.entries(obj)) {
    // console.log(key, value);
    var thetable;
    var conf;
    if (key == "confnets" || key == "discnets") {
      if (key == "confnets") {
        thetable = document.getElementById("conftable");
        conf = true;
      } else if (key == "discnets") {
        thetable = document.getElementById("disctable");
        conf = false;
      }

      var txt = thetable.innerHTML;
      console.log(thetable);
      var idn = 0;
      for (netdesc of value) {
        console.log(netdesc.ssid);
        var id;
        if (conf) id = "cf" + idn;
        else id = "df" + idn;
        idn++;

        txt +=
          "<tr><td>" +
          "<input type=checkbox ";
          if (!conf) txt+= "un";
          txt += "checked id=" +
          id +
          " name=conf /><label for=" +
          id +
          ">&nbsp;</label>" +
          "<input type=hidden name=ssid value=" +
          netdesc.ssid +
          "/></td>" +
          "<td>";
        if (conf) txt += "<a href=/config.netedit?ssid=" + netdesc.ssid + ">"; // Only confs
        if (netdesc.open) txt += "🔓";
        else txt += "🔒";
        txt += netdesc.ssid;
        if (conf) txt += "</a>"; // only confs
        txt += "</td></tr>";
      }
      thetable.innerHTML = txt;
    }
  }
  console.log(document);
}

function orsc() {
  if (this.readyState == XMLHttpRequest.DONE && this.status == 200) {
    console.log("response", this.responseText);
    var obj = JSON.parse(this.responseText);
    netdata(obj);
  }
}
