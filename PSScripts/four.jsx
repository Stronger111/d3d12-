
var input=new Folder("C:/Users/Administrator/Desktop/Diffuse")

var data=input.getFiles ("*.tga")

var output=new File("C:/Users/Administrator/Desktop/Diffuse/out/testPDF.pdf")

app.makePDFPresentation (data, output)

alert ("制作完成","任务", true)