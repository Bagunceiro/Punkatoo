var z = window.location.search;

const urlParams = new URLSearchParams(z);
var a = urlParams.keys();
for (k of a)
{
    console.log(k);
}
p =  urlParams.get("name");
console.log(p);
