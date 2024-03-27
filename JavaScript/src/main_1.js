(function() {
"use strict";
console.log("START ===");

var btnYTube = document.getElementById("btn_youtube");
var onClickYTube = function() {
    console.log("youTube button is clicked.");
}
btnYTube.addEventListener("click",onClickYTube);

var aSearch = document.getElementById("a_search");
console.log(aSearch.href);


/**
 * プロック文 {}
 * let および const の宣言は、宣言されたブロック文のスコープになることもあります。
 */
if(1.00) {
    function add(lhs, rhs) {
        return lhs + rhs;
    }
    let r = add(3, 6);
    console.log("r is " + r);

    const MY_ARRAY = ["HTML", "CSS"];
    MY_ARRAY.push("JAVASCRIPT");
    console.log(MY_ARRAY); // ['HTML', 'CSS', 'JAVASCRIPT'];
}
if(1.01) {
    const sales = "Toyota";

    function carTypes(name) {
      return name === "Honda" ? name : `Sorry, we don't sell ${name}.`;
    }
    
    const car = { myCar: "Saturn", getCar: carTypes("Honda"), getCar2: carTypes("AE 86"), special: sales };
    
    console.log(car.myCar);     // Saturn
    console.log(car.getCar);    // Honda
    console.log(car.special);   // Toyota    
    console.log(car.getCar2);
    /**
     * JS は凄いよね：）
     */
}
console.log("=== END");
})();