function fillFromJson(obj) {
  for (const [key, value] of Object.entries(obj)) {
    if (key == "selection") {
      var selects = document.getElementsByName(value);
      for (element of selects)
      {
        element.selected = "selected";
      }
    } else {
      var elements = document.getElementsByName(key);
      for (element of elements) {
        // var element = document.getElementById(key);
        if (element) {
          var tag = element.tagName;
          if (tag == "INPUT") {
            element.value = value;
          } else {
            element.innerHTML = value;
          }
        }
      }
    }
  }
}

function orsc() {
  if (this.readyState == XMLHttpRequest.DONE && this.status == 200) {
    console.log("response", this.responseText);
    var obj = JSON.parse(this.responseText);
    fillFromJson(obj);
  }
}
