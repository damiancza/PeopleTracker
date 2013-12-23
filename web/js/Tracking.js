var objects = [];

//Cargamos la imagen en el canvas
var canvas = document.getElementById("camcapt");
var context = canvas.getContext("2d");
var img =  document.getElementById("imgcamcapt");    

$(function() {
    var nframes = 0;
    
    $.getJSON( "/data/frpath.json", function( data ) {
    
        //Get the # of objets
        nframes = data.objects.length;
        
        if(nframes > 0)
        {
            $.each(data.objects, function(i,item){
                objects.push( item );

                $("#selectable").append("<li class=\"ui-widget-content\">Item :" + item.id + "</li>");
                
            });
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

    
    $("#imgcamcapt").attr("src", "/data/labeltestc.jpg?" + Math.random());
    
    img.onload = function () {
        context.lineWidth = 5;
        context.drawImage(img, 0, 0, 640, 480);
    }

});

function drawPath(index) {
    var obj = objects[index];
    var canvas = document.getElementById("camcapt");
    var context = canvas.getContext("2d");
    var img =  document.getElementById("thumbid");
    
    context.beginPath();
    context.lineWidth = 2;
    var firstFr = 0;
    //initial position
    var posx = 0;
    var posy = 0;
    
    for (var i=0;i<obj.frames.length;i++){ 
        if(i == 0){
            //keep the first frame number
            firstFr = obj.frames[i].frame;
            posx = obj.frames[i].x;
            posy = obj.frames[i].y;
            
            context.moveTo(obj.frames[i].x,obj.frames[i].y);
        }
        else{
            context.lineTo(obj.frames[i].x,obj.frames[i].y);
        }
    }

    //Dibujamos la imagen
    var idimg = index + 1;
    $("#thumbid").attr("src", "/data/" + obj.id.toString() + "-" + firstFr.toString() + ".jpg?" + Math.random());
    
    img.onload = function () {
        context.shadowColor = '#999';
        context.shadowBlur = 20;
        context.shadowOffsetX = 15;
        context.shadowOffsetY = 15;
        context.drawImage(img, posx, posy, this.width, this.height);
        
    }
    
    context.strokeStyle = '#ff0000';
    context.stroke();
    
}

$( "#selectable" ).selectable({
  stop: function(){
    var result = $( "#select-result" ).empty();
    $( ".ui-selected", this ).each(function(){
        var index = $( "#selectable li" ).index( this );
        
        result.append( " #" + ( index + 1 ) );
    
        drawPath(index);
        
    });
  }
});

