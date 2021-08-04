let menu_state = false;


var open_menu = false;
function changeBrightness() {
    if (!open_menu) {
        darken();
        open_menu = true;
    } else {
        brighten();
        open_menu = false;
    }
}

function brighten() {
    document.body.style.backgroundColor = "white";

    document.getElementById("main").style.filter = "brightness(100%)";
    //document.getElementById("open-sidebar").style.filter = "brightness(100%)";
}

function darken() {
    document.body.style.backgroundColor = "rgba(0,0,0,0.4)";

    document.getElementById("main").style.filter = "brightness(50%)";
    //document.getElementById("open-sidebar").style.filter = "brightness(50%)";
}

function controlSideBar() {
    let tempEl = document.createElement('div');
    tempEl.innerHTML = '&#60;';
    let currentState = document.getElementById("btn-sidebar");
    if (currentState.innerHTML === tempEl.innerHTML) {
        currentState.innerHTML = "&#62;";
        document.getElementById("SideBar").style.width = "0";
        document.getElementById("main").style.marginLeft = "0";
        brighten();
    } else {
        currentState.innerHTML = "&#60;";
        document.getElementById("SideBar").style.width = "275px";
        document.getElementById("main").style.marginLeft = "275px";
        if (menu_state === true) {
            darken();
        }
    }
}



