(function()
{
    var pdfPresetNames;    
    var title="Adobe Script Tutorial 2";    
    
    var g; //group
    var p; //panel
    var w; //window

    var btnFolderInput;
    var texFolderInput;
    var btnOk;
    var btnCancel;
    var listPdfPresets;    
    
    pdfPresetNames =app.pdfExportPresets.everyItem().name;    
    
    w=new Window("dialog",title);
    p=w.add("panel")
    
    g=p.add("group")
    
    btnFolderInput=g.add("button",undefined,"Folder....")
    texFolderInput=g.add("statictext",undefined,"",{truncate:"middle"});
    texFolderInput.preferredSize=[200,-1]
    p=w.add("panel",undefined,"Options")
    g=w.add("group")
    g.add("statictext",undefined,"PDF preset")
    listPdfPresets=g.add("dropdownlist",undefined,"哈哈哈")
    g.alignChildren="center"
    btnOk=g.add("button",undefined,"OK")
    btnCancel=g.add("button",undefined,"Cancel")
    
    btnFolderInput.onClick=function()
    {
        var f=Folder.selectDialog ()
        if(f)
        {
            texFolderInput.text=f.fullName
        }
    }

    btnOk.onClick=function()
    {
        w.close (1)
    }
     
     btnCancel.onClick=function()
     {
         w.close(0)
     }
   
    if(w.show()==1)
    {
        process();
        alert ("Done", title, false)
    }  

    function process()
    {
        alert("Do Some work")
    }
 })();