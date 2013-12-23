//Global vars
var iIn = 0;
var iOut = 0;

function LoadImage() {
    var canvas = document.getElementById("camcapt");
    var context = canvas.getContext("2d");
    var img =  document.getElementById("imgcamcapt");
 
    $("#imgcamcapt").attr("src", "/data/labeltestc.jpg?" + Math.random());

    var jqxhr = $.getJSON( "/data/data.json", function( data ) {
        //Check if the table is empty
        var table=document.getElementById("InOutTable");

        //If there´s no rows
        if( (data.in == 0 && data.out == 0) && 
             table.rows.length > 1)
        {
            for(var a = table.rows.length - 1; a > 0; a--)
            {
                table.deleteRow(a);
            }
        }
        
        if( (data.in > 0 || data.out > 0) && 
             table.rows.length == 1)
        {
            var elem = 0;
            
            elem =(data.in > data.out)?data.in:data.out;
            
            for(var i = 1; i <= elem ; i++)
            {
                var row=table.insertRow(1);
                var cell1=row.insertCell(0);
                var cell2=row.insertCell(1);

                if(i <= data.in)
                {
                    cell1.innerHTML="<img src=\"./data/In" + i + ".jpg?" + Math.random() + "\" class=\"thumbCounter\" >";
                }
                if(i <= data.out)
                {
                    cell2.innerHTML="<img src=\"./data/Out" + i + ".jpg?" + Math.random() + "\" class=\"thumbCounter\" >";
                }
            }
            
            iIn = data.in;
            iOut = data.out;
        }
        /*
            //fill the table
            
        }
        */
        //If changed, added a new element to the 
        if(data.in != iIn || data.out != iOut)
        {
            var row=table.insertRow(1);
            var cell1=row.insertCell(0);
            var cell2=row.insertCell(1);
            if(data.in > iIn)
            {
                cell1.innerHTML="<img src=\"./data/In" + data.in + ".jpg?" + Math.random() + "\" class=\"thumbCounter\" >";
            }
            
            if(data.out > iOut)
            {
                cell2.innerHTML="<img src=\"./data/Out" + data.out + ".jpg?" + Math.random() + "\" class=\"thumbCounter\" >";
            }
        }    
        
        iIn = data.in;
        iOut = data.out;
        $("#numberin").html(data.in);
        $("#numberout").html(data.out);
        //console.log( "elementos " + data.in + data.out );
    })
    .done(function() {
        //console.log( "second success" );
    })
    .fail(function() {
        //console.log( "error" );
    })
    .always(function() {
        //console.log( "complete" );
    });    
    
    img.onload = function () {
        context.lineWidth = 5;
        context.drawImage(img, 0, 0, 640, 480);
    }

}

//El intervalo para el refresco de la imagen
setInterval(function() {
        LoadImage();
    },200);


