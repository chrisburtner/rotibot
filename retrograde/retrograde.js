

function pad(num) {
    var s = "000000" + num;
    return s.substr(s.length - 6);
}


//GLOBALS

var filename = "";      //frame filename
var img = new Image(); //frames
var gfpImg = new Image();
var cherryImg = new Image();
var uvImg = new Image();
var contImg = new Image();

var bfActive = true;
var gfpActive=false;
var cherryActive=false;
var uvActive=false;
var contActive=false;

var framenumber = 0; //hold the currently displayed frame number
var expID = 0;
var lowthresh = 0;
var highthresh = 0;
var serverLocked = false; //the server busy lock
var totalframes = 0; //hold the total number of frames from description.txt
var lifespanrequested = false;  //hold a flag to see if you asked for the lifespan to be updated
var temperature=""; //holds the current temperature
var currzoom=1; //holds the current zoom

//define rectangle object
var rect = { x: 0, y: 0, w: 0, h: 0, f: 0 , t: 0, id: 0, c: 0}; // x,y, width, height,frame, type [ lifespan, analysis, background ], identifier, channel
var rects = [];
var elip = { x: 0, y: 0, w: 0, h: 0, a: 0, f: 0 };
var elips = [];
var polyp = { x: 0, y: 0 };
var polyg = {

	n: 0,
	
	polyp: [],

	f: 0,

	


};//end polyg object

var deadworm = { x: 0, y: 0, deathframe: 0, number: 0, daysold: -1, minutesold: -1 };
var deadworms = [];
var analrects = [];
var numrects = 0;
var numelips = 0;
var elipseangle=0.0;
var ctx;
var needtounlock = false;
var wormlist = [];
var shiftLock = false;
var toolstate;
var toolstyle;




function drawElips() {
	//console.log("numelipses: " + elips.length);
    for (i = 0; i < elips.length; i++) {
        if (framenumber >= elips[i].f) {
	    ctx.beginPath();
            ctx.strokeStyle = "magenta";
            ctx.lineWidth = 3;
            ctx.ellipse(elips[i].x, elips[i].y, elips[i].w, elips[i].h, elips[i].a, 0, 2 * Math.PI);
            ctx.stroke();
            ctx.font = "30px Arial";
            ctx.fillStyle = "magenta";
            ctx.fillText(i, elips[i].x, elips[i].y);
            ctx.strokeStyle = "green";
    	}
     }
}


function drawRects() {
	
    for (i = 0; i < rects.length; i++) {

	switch (rects[i].t){
		

		case "analysis":
			 if (framenumber == rects[i].f){
				ctx.strokeStyle="magenta";
				 ctx.strokeRect(rects[i].x, rects[i].y, rects[i].w, rects[i].h);
				ctx.font = "30px Arial";
				ctx.fillStyle="magenta";
		                ctx.fillText(rects[i].id, rects[i].x , rects[i].y-2 );
				
			}
		break;

		case "background":
			ctx.strokeStyle="aqua";
			 ctx.strokeRect(rects[i].x, rects[i].y, rects[i].w, rects[i].h);
		break;

		default:
		case "lifespan":
			 ctx.strokeStyle="green";
		       	 if (framenumber >= rects[i].f){
				 ctx.strokeRect(rects[i].x, rects[i].y, rects[i].w, rects[i].h);
			}//end if visible for lifespan
		break;

	}//end switch
    }
}//end draw rects


function drawDeadworms() {
    // console.log("drawingdeadworms");
    // console.log("deadworms.length=" + deadworms.length);
    //if (deadworms==null) return;
    for (i = 0; i < deadworms.length; i++) {
        //is worm visible
        if (framenumber >= deadworms[i].deathframe) {

            ctx.beginPath();
            ctx.strokeStyle = "red";
            ctx.lineWidth = 3;
            ctx.arc(deadworms[i].x, deadworms[i].y, 30, 0, 2 * Math.PI, 0);
            ctx.stroke();
            ctx.font = "30px Arial";
            ctx.fillStyle = "red";
            ctx.fillText(i, deadworms[i].x, deadworms[i].y);
            ctx.strokeStyle = "green";

        }//end if worm has died

    }//end for each deadworm
}//end drawdeadworms

function getToolSelect() {
	
	toolstyle = $('input[name=selectstyle]:checked').val()
	//console.log("toolstyle: " + toolstyle);
	toolstate = $('input[name=selectstate]:checked').val()
	//console.log("toolstate: " + toolstate);
	
}//end getToolSelect

function drawChannels() {
	var mrect = canvas.getBoundingClientRect();
	ctx.clearRect(0, 0, 10000, 10000);


	 bfActive = document.querySelector("#BFchan").checked;
	gfpActive = document.querySelector("#GFPchan").checked;
	cherryActive= document.querySelector("#CHERRYchan").checked;
	uvActive = document.querySelector("#UVchan").checked; 
	contActive = document.querySelector("#CONTchan").checked; 
	getToolSelect();

	if (bfActive) ctx.drawImage(img, 0, 0);
	if (gfpActive) ctx.drawImage(gfpImg, 0, 0);
	if (cherryActive) ctx.drawImage(cherryImg, 0, 0);
	if (uvActive) ctx.drawImage(uvImg, 0, 0);
	if (contActive) ctx.drawImage(contImg, 0, 0);
	
	

}//end drawChannels

function drawExpID() {
    ctx.font = "30px Arial";
    ctx.fillStyle = "cyan";
    ctx.fillText("ExpID = " + expID, 30, 30);
}//end drawExpID

function drawTemp(){
    ctx.font = "30px Arial";
    ctx.fillStyle = "cyan";
    ctx.fillText("Temp=" + temperature + "C", 500, 30);
}//end drawTemp

function drawLock() {
    ctx.font = "30px Arial";
    ctx.fillStyle = "cyan";
   // ctx.clearRect(1000, 0, 1500, 100);
    if (shiftLock) {

       // ctx.fillText("Bounding Box Disabled", 1000, 30);
	 document.getElementById("shiftlockerID").innerHTML = "ON";
    } else {
       // ctx.fillText("Bounding Box Enabled", 1000, 30);
	 document.getElementById("shiftlockerID").innerHTML = "OFF";
    }
   
    
}

function killWorm(tx, ty) {
    var thiswormnum;
    if (tx == undefined || ty == undefined) return;
    if (deadworms == undefined) thiswormnum = 0; else thiswormnum = deadworms.length;
    var thisworm = { x: tx, y: ty, deathframe: framenumber, number: thiswormnum, daysold: -1, minutesold: -1 };
    deadworms.push(thisworm);
    console.log("made worm num " + deadworms.length + " at " + tx + "," + ty + "," + framenumber);

}//end killworm

function getFileDate(filename) {
    var xhr = $.ajax({
        url: filename,
        success: function (response) {
            console.log("last modified: " + xhr.getResponseHeader("Last-Modified"));
        }
    });
}//end function getfiledate


function LoadWormList() {
    var eachworm;
    var rnum = Math.random();
    filename = "/wormbot/" + expID + "/wormlist.csv?v=" + rnum;
    $.ajax({
        url: filename,
        success: function (data) {
            if (data.length < 1) return;
            if (deadworms.length > 0) { console.log("erase deadworms length=" + deadworms.length); while (deadworms.length > 0) deadworms.pop(); }

            eachworm = data.split('\n');
            console.log("eachworm.length:" + eachworm.length);
            for (i = 0; i < eachworm.length; i++) {
                var thisworm;
                console.log("eachworm[" + i + ":" + eachworm[i]);
                thisworm = eachworm[i].split(',');
                if (thisworm.length < 5) continue;
                var wormstruct = { x: parseInt(thisworm[0]), y: parseInt(thisworm[1]), deathframe: parseInt(thisworm[2]), number: parseInt(thisworm[3]), daysold: parseFloat(thisworm[4]), minutesold: parseFloat(thisworm[5]) };
                console.log(wormstruct);
                deadworms.push(wormstruct);
                console.log("i=" + i);
                console.log("load worms deadworms length:" + deadworms.length);
                getFileDate(filename);
            }//end for each worm line
            doUpdateWormList();

        }
    });


}//end load wormlist



function LoadFrame() {
    ctx.clearRect(0, 0, 10000, 10000);
    if (framenumber < 0) framenumber = 0;
    if (framenumber > 999999) framenumber = 999999;
    filename = "/wormbot/" + expID + "/frame" + pad(framenumber) + ".png";
    img.src = filename;
    filename = "/wormbot/" + expID + "/GFP" + pad(framenumber) + ".png";
    gfpImg.src = filename;
    filename = "/wormbot/" + expID + "/UV" + pad(framenumber) + ".png";
    uvImg.src = filename;
    filename = "/wormbot/" + expID + "/CHERRY" + pad(framenumber) + ".png";
    cherryImg.src = filename;
    var rnum = Math.random()	
    filename = "/wormbot/" + expID + "/current_contrast.png?v=" + rnum;
    contImg.src = filename;	
    img.onload = function () {
	
        
	drawChannels();
	document.getElementById("BFchan").disabled= false;
    };

    img.onerror = function (){
	//disable that channel selector if image error
	document.getElementById("BFchan").disabled= true;

	};//end on error
	
	  gfpImg.onload = function () {
	 
	document.getElementById("BFchan").disabled= false;
	drawChannels();
      
    };
	gfpImg.onerror = function (){
	//disable that channel selector
	document.getElementById("GFPchan").disabled= true;

	};//end on error

  	cherryImg.onload = function () {
	document.getElementById("CHERRYchan").disabled= false;
	drawChannels();
      
    };

	cherryImg.onerror = function (){
	//disable that channel selector
	document.getElementById("CHERRYchan").disabled= true;

	};//end on error

	 uvImg.onload = function () {
	document.getElementById("UVchan").disabled= false;
	drawChannels();
      
    };

	uvImg.onerror = function (){
	//disable that channel selector
	document.getElementById("UVchan").disabled= true;

	};//end on error

	 contImg.onload = function () {
	document.getElementById("CONTchan").disabled= false;
	drawChannels();
      
    };

	contImg.onerror = function (){
	//disable that channel selector
	document.getElementById("CONTchan").disabled= true;

	};//end on error

    drawChannels();
    drawRects();
    drawElips();
    drawDeadworms();
    drawExpID();
    updateFrameField();
	

}//end load frame


function updateFrameField() {
	//update the frame field
	document.getElementById("currframeID").innerHTML = pad(framenumber); 
	document.getElementById("currexpID").innerHTML = expID; 
	document.getElementById("zoomID").innerHTML = currzoom.toPrecision(3); 

}//end update frame field


document.onkeydown = function (e) {

    switch (e.key) {
        case 'ArrowUp':
            e.preventDefault();
            console.log("arrowup");
            framenumber += 10;
            if (framenumber >= totalframes) framenumber = totalframes - 1;
            LoadFrame();
            // up arrow
            break;
        case 'ArrowDown':
            // down arrow
            e.preventDefault();
            console.log("arrowdwn");
            framenumber -= 10;
            if (framenumber < 0) framenumber = 0;
            LoadFrame();
            break;
        case 'ArrowLeft':
            // left arrow
            e.preventDefault();
            framenumber--;
            if (framenumber < 0) framenumber = 0;
            LoadFrame();
            console.log("arrowLeft");

            break;
        case 'ArrowRight':
            // right arrow
            e.preventDefault();
            framenumber++;
            if (framenumber >= totalframes) framenumber = totalframes - 1;
            LoadFrame();
            console.log("arrowright");
        case 'Shift':
            shiftLock = !shiftLock;
            drawLock();
    }
};


function getLastFile() {
    var lines;
    filename = "/wormbot/" + expID + "/description.txt";
    $.ajax({
        url: filename,
        success: function (data) {
            lines = data.split('\n');
            for (i = 0; i < lines.length; i++) { console.log("line[" + i + ":" + lines[i]); }
            framenumber = lines[6];
            totalframes = framenumber;
            console.log("last frame is: " + framenumber);
            framenumber = framenumber - 1; //off by one
            LoadFrame();

            return lines[6];
        }
    });
    //return the 6th line which holds the lastframe
}//end getLastFile



function checkLock() {
    //check the server lock to see if it's ready
    $.ajax({
        url: '/wormbot/serverlock.lock?v=' + Math.random() ,
        type: 'HEAD',
        error: function () {
            serverLocked = false;
            // $('#btn').addClass("btn-open");
            // $('#btn').removeClass("followed btn-locked");
            // $('#btn').html("Server Ready");
            $('#loader1').css("display", "none");

            if (needtounlock) {
                LoadWormList();
                needtounlock = false;
            }

        },
        success: function () {
            serverLocked = true;
            $('#loader1').css("display", "inline-block");
            // $('#btn').addClass("followed btn-locked");
            // $('#btn').html("Processing...Please wait");
            //needtounlock=true;

        }
    });


}//end checkLock


function getMousePos(canvas, evt) {
    var brect = canvas.getBoundingClientRect();
    return {
        x: evt.clientX - brect.left,
        y: evt.clientY - brect.top
    };var url = "/wormbot/" + expID + "/current_contrast.png?v=" + Math.random();

}//end get mouse position



function popContourPreview() {
    var rnum = Math.random();
    var url = "/wormbot/" + expID + "/current_contrast.png?v=" + rnum;
    var image = new Image();
    image.src = url;
    document.getElementById("preview-canvas").getContext('2d').drawImage(image, 0, 0);
    
    //window.open(url, 'Image', 'width=1920,height=1080,resizable=1');

}



function deleteObject(x, y) {
    //scan through objects and delete them if found to have middle clicked on them
    var i = 0;
    for (i = 0; i < rects.length; i++) {
        if (x > rects[i].x && x < rects[i].x + rects[i].w
            && y > rects[i].y && y < rects[i].y + rects[i].h) {
            rects.splice(i, 1);
            return;

        }//end if hit the rectangle
    }
    var j = 0;

	for (i = 0; i < elips.length; i++) {

		if (  ((((x - elips[i].x)**2)/ elips[i].w **2) + (((y - elips[i].y)**2)/ elips[i].h **2)) < 1) {
			elips.splice(i,1);
			return;
		}//end if inside elipse

	}//end for elips

    console.log("deadworms length:" + deadworms.length);
    console.log("delete obj x,y:" + x + "," + y);

    for (j = 0; j < deadworms.length; j++) {



        if (x > (deadworms[j].x - 20) && x < (deadworms[j].x + 20) && y > (deadworms[j].y - 20) && y < (deadworms[j].y + 20)) {
            console.log("deleted j:" + j);
            deadworms.splice(j, 1);
            return;

        }//end if hit the rectangle
    }




}//end delete object

function makeDataPanel() {
    // var leftDiv = document.createElement("div"); //Create left div
    // leftDiv.id = "left"; //Assign div id
    // leftDiv.setAttribute("style", "float:left; width:25%; line-height: 26px; text-align:left; font-size:16pt; padding-left:8px; height:26px;");
    // leftDiv.style.background = "#999999";

    // var leftDiv2 = document.createElement("div"); //Create left div
    // leftDiv2.id = "left"; //Assign div id
    // leftDiv2.setAttribute("style", "float:left; width:25%; line-height: 26px; text-align:left; font-size:16pt; padding-left:8px; height:26px;");
    // leftDiv2.style.background = "#CCCCCC";

    var movieDiv, lpDiv, msDiv;
    movieDiv = document.getElementById("movie-div");
    lpDiv = document.getElementById("lp-div");
    msDiv = document.getElementById("ms-div");

    a = document.createElement('a');
    var rnum = Math.random();
    a.href = '/wormbot/' + expID + "/" + expID + ".avi?v=" + rnum;
    a.innerHTML = "Download Current Movie"
    movieDiv.appendChild(a);
    // document.body.appendChild(leftDiv);
    b = document.createElement('a');
    b.href = '/wormbot/' + expID + "/lifespanoutput_" + expID + ".csv?v=" + rnum;
    b.innerHTML = "Download Current Data"
    lpDiv.appendChild(b);
    c = document.createElement('a');
    c.href = '/wormbot/' + expID + "/lifespanoutput_" + expID + ".csv?v=" + rnum;
    c.innerHTML = "Download Current Data"
    msDiv.appendChild(c);
    // document.body.appendChild(leftDiv2);
}


function doUpdateWormList() {

    var deadwormsstring = JSON.stringify(deadworms);
    var formE1 = document.forms.updateForm1;
    var formData1 = new FormData(formE1);
    var startmovie = formData1.get('moviestart');
    var stopmovie = formData1.get('moviestop');
    var checkboxbuildmovie = formData1.get('buildMovie');
    var moviechannel = formData1.get('mchan');
    var mres = formData1.get('mres');	
    var drawDeadWorms = formData1.get('drawDead');	
	//console.log("movie channel: " + moviechannel);	

    var formE2 = document.forms.updateForm2;
    var formData2 = new FormData(formE2);
    highthresh = formData2.get('upperthresh');
    lowthresh = formData2.get('lowerthresh');
    var checkboxUpdateContours = formData2.get('updatecontours');
    var bgstyle= 0;
    

	 var analrects  = rects.filter(arec => arec.t === 'analysis');
		console.log(analrects);
            var jsonAnalRects = JSON.stringify(analrects);
		console.log(jsonAnalRects);


    $.ajax({
        type: "POST",
        url: "/cgi-bin/wormlistupdater",
        data: {
            "deadworms": deadwormsstring, "analrects": jsonAnalRects, "expID": expID, "startmovie": startmovie, "stopmovie": stopmovie,
            "buildMovie": checkboxbuildmovie, "highthresh": highthresh, "mchan" : moviechannel,
            "lowthresh": lowthresh, "updatecontours": checkboxUpdateContours,
            "currframe": framenumber, "drawDead": drawDeadWorms, "mres": mres, "bgstyle": bgstyle
        },
        success: function () {

            //alert(data);
            alert("Worm List Updated");
	    LoadFrame();
	    //redraw();		

        }
    });




}//end dod updatewormlist






$(window).load(function () {



    document.addEventListener('contextmenu', event => event.preventDefault());

    var myVar;

    function myFunction() {
        myVar = setInterval(checkLock, 10000);
    }


    myFunction();
    //check to see if the server is busy with a job
    checkLock();



    //setup the buttons
    var delBtn = document.createElement("button");
    var deleteTxt = document.createTextNode("Update Server Data");
    delBtn.className = "side-btn";
    delBtn.appendChild(deleteTxt);

    var getlifespanContainer = document.createElement("div");
    var lifespanBtn = document.createElement("button");
    var lifespanTxt = document.createTextNode("Get Lifespan");
    lifespanBtn.appendChild(lifespanTxt);
    lifespanBtn.id = "lifespan";
    lifespanBtn.className = "side-btn";
    getlifespanContainer.appendChild(lifespanBtn);
    var loadDiv = document.createElement("div");
    loadDiv.className = "load-div";

    var loader = document.createElement("span");
    loader.id = "loader1";
    loader.className = "loader";
    loadDiv.appendChild(loader);
    getlifespanContainer.appendChild(loadDiv);


    var gotoBtn = document.createElement("button");
    var gotoTxt = document.createTextNode("Goto Frame");
    gotoBtn.appendChild(gotoTxt);
    gotoBtn.className = "side-btn";


	//add listeners for channel checkboxes
	var bfE = document.querySelector("#BFchan");

	bfE.addEventListener( 'change', function() {
	   	redraw();
	});
	var gfpE = document.querySelector("#GFPchan");

	gfpE.addEventListener( 'change', function() {
	   	redraw();
	});
	var cherryE = document.querySelector("#CHERRYchan");

	cherryE.addEventListener( 'change', function() {
	   	redraw();
	});
	var uvE = document.querySelector("#UVchan");

	uvE.addEventListener( 'change', function() {
	   	redraw();
	});
	var contE = document.querySelector("#CONTchan");

	contE.addEventListener( 'change', function() {
	   	redraw();
	});
   


    //prompt for initial values

     document.querySelector("#BFchan").checked=true;
     document.querySelector("#GFPchan").checked=false;
     document.querySelector("#CHERRYchan").checked=false;
     document.querySelector("#UVchan").checked=false; 	

    expID = prompt("Enter the experiment ID");
    lowthresh = prompt("Enter the lower brightness threshold 0-255", 80);
    highthresh = prompt("Enter the upper brightness threshold 0-255", 210);

    //should go to last frame
    getLastFile();

    //load the exisiting wormlist
    LoadWormList();

    //add buttons
    var body = document.getElementsByTagName("body")[0];
    document.getElementById("control-panel").appendChild(delBtn);
    document.getElementById("lp-div").appendChild(getlifespanContainer);
    document.getElementById("fn-div").appendChild(gotoBtn);
    



    delBtn.addEventListener("click", function () {
	doUpdateWormList();
	//LoadFrame();
	//redraw();
    });

    gotoBtn.addEventListener("click", function () {
        framenumber = parseInt(prompt("Goto Frame Number"));
	LoadFrame();
        redraw();

    });//end add event listener

    lifespanBtn.addEventListener("click", function () {
        if (serverLocked == false) {
	    var liferects  = rects.filter(arec => arec.t === 'lifespan');
		console.log(liferects);
            var outputRects = JSON.stringify(liferects);
		console.log(outputRects);
            $.ajax({
                type: "POST",
                url: "/cgi-bin/cgiccretro",
                data: { "name": outputRects, "expID": expID, "lowthresh": lowthresh, "highthresh": highthresh },
                success: function (data) {
                    alert(data);

                }
            });
            serverLocked = true;
            // $('#btn').addClass("followed btn-locked");
            // $('#btn').html("Processing...Please wait");
            $('#loader1').css("display", "inline-block");
            console.log(liferects);
            needtounlock = true;

        }//end if not locked
        }//end function
    );

    makeDataPanel();

//

    // get references to the canvas and context
    var canvas = document.getElementById("canvas");
    ctx = canvas.getContext("2d");
	


    // style the context
    ctx.strokeStyle = "green";
    ctx.lineWidth = 3;
	//set ctx params
	ctx.globalCompositeOperation = 'screen';



    // this flage is true when the user is dragging the mouse
    var isDown = false;

    // these vars will hold the starting and present mouse position
    var startX;
    var startY;
    var mouseX;
    var mouseY;

    function handleMouseDown(e) {
        e.preventDefault();
        e.stopPropagation();

        var brect = canvas.getBoundingClientRect();

        // console.log(brect.right + " " + brect.left + " " + brect.top + " " + brect.bottom);
        // console.log(canvas.width + " " + canvas.height + " " + brect.width + " " + brect.height);
        var x_scale = canvas.width / brect.width * parseInt(e.clientX - brect.left);
        var y_scale = canvas.height / brect.height * parseInt(e.clientY - brect.top);
        var temp = ctx.transformedPoint(x_scale, y_scale)

        // var x_scale = e.clientX;
        // var y_scale = e.clientY;
        if (shiftLock) return;
        switch (e.which) {

            case 1:   //left mouse button down
                // save the starting x/y of the rectangle
                startX = temp.x;
                startY = temp.y;

                isDown = true; //dragging a rectangle
                break;
            case 2:  //middle mouse button down
                //console.log("middle mouse pressed");
                deleteObject(temp.x, temp.y);

                break;
            case 3: //right mouse button down
                //console.log("right mouse pressed");
                killWorm(temp.x, temp.y);

                break;


        }//end switch

    }

    function addNewRect(theType) {
        isDown = false;
        if (shiftLock) return;
	
        numrects++;
        //standardize the rect to positive width and height
        var swap = 0;
	var rwidth=0;
	var rheight=0;

	if (toolstyle == "rect"){
		if (mouseX - startX < 0) {
		    swap = startX;
		    startX = mouseX;
		    mouseX = swap;
		}
		rwidth = mouseX - startX;
		if (mouseY - startY < 0) {
		    swap = startY;
		    startY = mouseY;
		    mouseY = swap;
		}
		rheight = mouseY - startY

		if (mouseX - startX == 0 || mouseY - startY == 0) return; //no zero area rectangle allowed
	}//end if rect 
	else if (toolstyle== "fixed"){
		rwidth = Math.abs(parseInt(document.getElementById("fixedW").value));
		rheight = Math.abs(parseInt(document.getElementById("fixedH").value));

	}//end if fixed
	 

	var rectID = document.getElementById("analID").value;
	var rectChan = document.getElementById("analChan").value;  

        var newRect = { name: numrects, x: startX, y: startY, w: rwidth, h: rheight, f: framenumber, t: theType, id: rectID, c: rectChan };
        rects.push(newRect);
    }

   function addNewElipse() {
        isDown = false;
        if (shiftLock) return;
        numelips++;
        //standardize the rect to positive width and height
        var swap = 0;
        if (mouseX - startX < 0) {
            swap = startX;
            startX = mouseX;
            mouseX = swap;
        }
        if (mouseY - startY < 0) {
            swap = startY;
            startY = mouseY;
            mouseY = swap;
        }

        var newelips = { name: numelips, x: startX, y: startY, w: mouseX - startX, h: mouseY - startY, a: elipseangle, f: framenumber };
        elips.push(newelips);
    }


    /*
                function removeLastWorm(){
            numrects--;
        rects.pop();
    }//end remove last worm
    */

    function handleMouseUp(e) {
        e.preventDefault();
        e.stopPropagation();

	
       
	
	//console.log("bfactive:" + bfActive);


	
	

        // the drag is over, clear the dragging flag
        switch (e.which) {
            case 1:  // left mouse button
                if (toolstyle == "rect" || toolstyle == "fixed"){
			addNewRect(toolstate);

		}//end if rect 
		 if (toolstyle == "elipse") addNewElipse();
                break;
            case 2: //middle mouse button
                // clear the canvas
                
		drawChannels();
                drawRects();
                drawDeadworms();
                break;

            case 3: //right mouse button
                break;

        }//end switch
    }




    function handleMouseOut(e) {
        e.preventDefault();
        e.stopPropagation();

        // the drag is over, clear the dragging flag
        isDown = false;
    }




    function handleMouseMove(e) {
        e.preventDefault();
        e.stopPropagation();

        var brect = canvas.getBoundingClientRect();
	

        var x_scale = canvas.width / brect.width * parseInt(e.clientX - brect.left);
        var y_scale = canvas.height / brect.height * parseInt(e.clientY - brect.top);
        // get the current mouse position
        var temp = ctx.transformedPoint(x_scale, y_scale);
        mouseX = temp.x;
        mouseY = temp.y;

        // mouse pos debugging
        // var mouseOutput = "mouse.x:" + mouseX + " mouse.y:" + mouseY;
        //  console.log(mouseOutput);
        if (shiftLock) return;

        // clear the canvas
        //ctx.drawImage(img, 0, 0);
	drawChannels();



        // calculate the rectangle width/height based
        // on starting vs current mouse position
       
	
	$(document).keydown(function(event) {
    		if (event.which == "17")
       		 elipseangle+=0.0001;
    
	 });

	var width;
	var height;


	if (toolstyle == 'rect') {
		width = mouseX - startX;
		height = mouseY - startY;
	} else if (toolstyle == 'fixed') {
		width = Math.abs(parseInt(document.getElementById("fixedW").value));
		height = Math.abs(parseInt(document.getElementById("fixedH").value));
		startX= mouseX;
		startY=mouseY;
	}//end if fixed

        // draw a new rect from the start position
        // to the current mouse position
        if (isDown) {

		if (toolstyle == 'rect' || toolstyle == 'fixed') {
			switch(toolstate) {
				case 'lifespan':
					 ctx.strokeStyle = "green";
				break;
				case 'analysis':
					 ctx.strokeStyle = "magenta";
				break;
				case 'background':
					 ctx.strokeStyle = "aqua";
				break;
				
			}//end switch
			ctx.strokeRect(startX, startY, width, height);

			
		}
		else if (toolstyle == 'elipse') {


				
				 ctx.beginPath();
			    ctx.strokeStyle = "magenta";
			    ctx.lineWidth = 5;
				
			    ctx.ellipse(startX, startY, width, height, elipseangle, 0, 2 * Math.PI);
			    ctx.closePath()
			    ctx.stroke();
			}
	}//end if dragging
        drawRects();
	drawElips();
        drawDeadworms();
        drawExpID();
    }


    //listen for keyboard events

    // listen for mouse events
    $("#canvas").mousedown(function (e) {
        handleMouseDown(e);
	
    });
    $("#canvas").mousemove(function (e) {
        handleMouseMove(e);
    });
    $("#canvas").mouseup(function (e) {
        handleMouseUp(e);
	redraw();
    });
    $("#canvas").mouseout(function (e) {
        handleMouseOut(e);
    });

    trackTransforms(ctx);

    function redraw() {

        // Clear the entire canvas
        var p1 = ctx.transformedPoint(0, 0);
        var p2 = ctx.transformedPoint(canvas.width, canvas.height);
        ctx.clearRect(p1.x, p1.y, p2.x - p1.x, p2.y - p1.y);

        ctx.save();
        ctx.setTransform(1, 0, 0, 1, 0, 0);
        ctx.clearRect(0, 0, canvas.width, canvas.height);
        ctx.restore();

        //ctx.drawImage(img, 0, 0);
	drawChannels();

	//update the frame field
	updateFrameField();
	

        drawRects();
	drawElips();
        drawDeadworms();
        drawExpID();
        drawTemp();
        drawLock();
    }
    redraw();

    var lastX = canvas.width / 2, lastY = canvas.height / 2;
    var brect = canvas.getBoundingClientRect()
    lastX *= canvas.width / brect.width;
    lastY *= canvas.height / brect.height;
    var dragStart, dragged;

    canvas.addEventListener('mousedown',function(evt){
        document.body.style.mozUserSelect = document.body.style.webkitUserSelect = document.body.style.userSelect = 'none';
        lastX = evt.offsetX || (evt.pageX - canvas.offsetLeft);
        lastY = evt.offsetY || (evt.pageY - canvas.offsetTop);
        var brect = canvas.getBoundingClientRect()
        lastX *= canvas.width / brect.width;
        lastY *= canvas.height / brect.height;
        dragStart = ctx.transformedPoint(lastX,lastY);
        dragged = false;
    },false);

    canvas.addEventListener('mousemove', function (evt) {
        lastX = evt.offsetX || (evt.pageX - canvas.offsetLeft);
        lastY = evt.offsetY || (evt.pageY - canvas.offsetTop);
        var brect = canvas.getBoundingClientRect()
        lastX *= canvas.width / brect.width;
        lastY *= canvas.height / brect.height;
        if (!shiftLock) return;
        dragged = true;

        if (dragStart) {
            var pt = ctx.transformedPoint(lastX, lastY);
            ctx.translate(pt.x - dragStart.x, pt.y - dragStart.y);
            
            redraw();
        }
    }, false);

    canvas.addEventListener('mouseup',function(evt){
        dragStart = null;
    },false);
    var scaleFactor = 1.05;

    var zoom = function (clicks) {
        var pt = ctx.transformedPoint(lastX, lastY);
        ctx.translate(pt.x, pt.y);
        var factor = Math.pow(scaleFactor, clicks);
	//console.log(factor * currzoom);
	currzoom = currzoom * factor;
        ctx.scale(factor, factor);
        ctx.translate(-pt.x, -pt.y);
        redraw();
    }

    var handleScroll = function (evt) {
        var delta = evt.wheelDelta ? evt.wheelDelta / 40 : evt.detail ? -evt.detail : 0;
        if (delta) zoom(delta);
        return evt.preventDefault() && false;
    };
    




    canvas.addEventListener('DOMMouseScroll', handleScroll, false);
    canvas.addEventListener('mousewheel', handleScroll, false);

}); //end onload

function trackTransforms(ctx) {
    var svg = document.createElementNS("http://www.w3.org/2000/svg", 'svg');
    var xform = svg.createSVGMatrix();
    ctx.getTransform = function () { return xform; };

    var savedTransforms = [];
    var save = ctx.save;
    ctx.save = function () {
        savedTransforms.push(xform.translate(0, 0));
        return save.call(ctx);
    };

    var restore = ctx.restore;
    ctx.restore = function () {
        xform = savedTransforms.pop();
        return restore.call(ctx);
    };

    var scale = ctx.scale;
    ctx.scale = function (sx, sy) {
        xform = xform.scaleNonUniform(sx, sy);
        return scale.call(ctx, sx, sy);
    };

    var rotate = ctx.rotate;
    ctx.rotate = function (radians) {
        xform = xform.rotate(radians * 180 / Math.PI);
        return rotate.call(ctx, radians);
    };

    var translate = ctx.translate;
    ctx.translate = function (dx, dy) {
        xform = xform.translate(dx, dy);
        return translate.call(ctx, dx, dy);
    };

    var transform = ctx.transform;
    ctx.transform = function (a, b, c, d, e, f) {
        var m2 = svg.createSVGMatrix();
        m2.a = a; m2.b = b; m2.c = c; m2.d = d; m2.e = e; m2.f = f;
        xform = xform.multiply(m2);
        return transform.call(ctx, a, b, c, d, e, f);
    };

    var setTransform = ctx.setTransform;
    ctx.setTransform = function (a, b, c, d, e, f) {
        xform.a = a;
        xform.b = b;
        xform.c = c;
        xform.d = d;
        xform.e = e;
        xform.f = f;
        return setTransform.call(ctx, a, b, c, d, e, f);
    };

    var pt = svg.createSVGPoint();
    ctx.transformedPoint = function (x, y) {
        pt.x = x; pt.y = y;
        return pt.matrixTransform(xform.inverse());
    }
}
