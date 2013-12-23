$(function() {
    var imges = [];
    var nframes = 0;
    var lineChartData = {
          labels : [/*"13:00pm","February","March","April","May","June","13:15 pm"*/],
                 datasets : [
                              {
                                      fillColor : "rgba(220,220,220,0.5)",
                                      strokeColor : "rgba(220,220,220,1)",
                                      pointColor : "rgba(220,220,220,1)",
                                      pointStrokeColor : "#fff",
                                      data : [/*65,59,90,81,56,55,40*/]
                              }
                        ]
                        
                };
    
    $.getJSON( "/data/path1.json", function( data ) {
        var interval = 5;
        var icounter = 0;
        var biggerObjects = 0;
        
        //Get the # of frames
        nframes = data.frames.length;
        
        if(nframes > 0)
        {
            $.each(data.frames, function(i,item){
                imges.push( item );
                if(icounter == interval)
                {
                    lineChartData.labels.push("");
                    lineChartData.datasets[0].data.push(biggerObjects);
                    icounter = 0;
                    biggerObjects = 0;
                }
                else
                {
                    if(item.objects > biggerObjects)
                    {
                        biggerObjects = item.objects;
                    }
                    icounter++;
                }
            });
            var myLine = new Chart(document.getElementById("line").getContext("2d")).Line(lineChartData);
        }

    })
    .done(function() {
        console.log( "second success" );
    })
    .fail(function() {
        console.log( "error" );
    })
    .always(function() {
        console.log( "complete" );
    });    

    function getMousePos(canvas, e) {
        var rect = canvas.getBoundingClientRect();
        return {x: e.clientX - Math.round(rect.left), y: e.clientY - Math.round(rect.top), size: Math.round(rect.right - rect.left)};
    }

    function followMiniImage(e) {
        /*var canvas = document.getElementById("line");
        var context = canvas.getContext("2d");
        var img =  document.getElementById("imgcamcapt");
        
        context.drawImage(img, e.clientX, e.clientY -200, 64, 48);*/
        var canvas = document.getElementById("line");

        var pos = getMousePos(canvas,e);
    
        var offsetscr = 1;
        
        if(pos.x > offsetscr)
        {
            //Get percent of the pos
            var position = (pos.x + offsetscr) / (pos.size - offsetscr) * 100;
            var imgpos = nframes * position /100;
            
            console.log("pos :" + (Math.round(imgpos) - 1) + ", image:" + imges[Math.round(imgpos) - 1].imagesrc);
            
            $("#imgcamcapt").attr("src", "/data/" + imges[Math.round(imgpos) - 1].imagesrc );
        }
        /*
        
        $("#imgcamcapt")[0].style.left = e.clientX + "px";
        $("#imgcamcapt")[0].style.top = (e.clientY + 200) + "px";
        */
        
    }

    function showMiniImage(e) {
      $("#imgcamcapt")[0].style.display = "block";
    }

    function hideMiniImage(e) {
      $("#imgcamcapt")[0].style.display = "none";
    }
    
    //Eventos
    $("#line").mousemove(followMiniImage);

    //$("#line").mouseenter(showMiniImage);
    
    //$("#line").mouseleave(hideMiniImage);

    
        
});
