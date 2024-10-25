import os
import pickle
import sys

#from config import
rdc_file_path="D:\Project\GraphicsTools\jeitu\Fur.rdc"

FetchShaderId=True
NeedReparse=False

#Import renderdoc if not already imported (e.g in theUI)
if 'renderdoc' not in sys.modules and '_renderdoc' not in sys.modules:
    import renderdoc

#Alias renderdoc for legibility
rd=renderdoc

def loadCapture(filename):
    #Open a Capture file handle
    cap=rd.OpenCaptureFile()
    
    #Open a particular file - see also OpenBuffer to load from memory
    status=cap.OpenFile(filename, "", None)
   
    #Make sure the file opened successfully
    if status != rd.ResultCode.Succeeded:
        raise RuntimeError("Couldn't open file: "+str(status))
    
    #Make sure we can replay
    if not cap.LocalReplaySupport(): 
        raise RuntimeError("Capture can't be replayed")
    
    #Initialise the replay
    status,controller=cap.OpenCapture(rd.ReplayOptions(),None)
    
    if status!=rd.ResultCode.Succeeded: 
        raise RuntimeError("Couldn't initialise replay: "+str(status))
    return cap,controller

def iter_texture(iter_texture,iter_events):
    for t_id in iter_texture:
        add_e_ids=[n for n in events_per_tex.get(t_id,[]) if n not in iter_events]
        iter_events+=add_e_ids
        #print("event:",iter_events)
        
        for e_id in add_e_ids:
            add_e_ids=[n for n in textures_per_dc.get(e_id,[]) if n not in iter_events]
            #print("texture:",iter_texture)
            iter_texture(add_t_ids,iter_events)
            iter_textures+=add_t_ids

def get_texture_usage(controller):
    events_per_tex={}
    textures_per_dc={}
    for r in controller.GetResources():
        events_per_tex[int(r.resourceId)]=[]
        for event in controller.GetUsage(r.resourceId): 
            events_per_tex[int(r.resourceId)].append(event.eventId)
            if textures_per_dc.get(event.eventId,None) is None:
                res_list=[]
                res_list.append(int(r.resourceId))
                textures_per_dc[event.eventId]=res_list
            else:
                textures_per_dc[event.eventId].append(int(r.resourceId))
    stats_info['useTextureCount']=len(events_per_tex)
    return events_per_tex,textures_per_dc

def save_textures(controller,save_dir_path):
    saved_texture={}
    if not os.path.exists(save_dir_path):
        print(save_dir_path)
        os.mkdir(save_dir_path)
    texsave=rd.TextureSave()
    for r in controller.GetResources():
        if r.type is rd.ResourceType.Texture:
            texture=r.resourceId
            filename='%d,%s.jpg' % (int(texture),r.name)
            saved_texture[int(texture)]={"name":r.name,"path":'.\\texture\\'+filename }

            if not os.path.exists(filename):
                texsave.resourceId=texture
                texsave.alpha=rd.AlphaMapping.BlendToCheckerboard
                texsave.mip=0
                texsave.slice.sliceIndex=0
                texsave.destType=rd.FileType.JPG
                controller.SaveTexture(texsave,os.path.join(save_dir_path,filename))
    return saved_texture

def save_shaders(controller,save_dir_path):
    if not os.path.exists(save_dir_path):
        os.makedirs(save_dir_path)
    for r in controller.GetResources():
        if r.type is rd.ResourceType.Shader:
            pipeline=r.resourceId
            shader=r.resourceId
            entry=controller.GetShaderEntryPoints(shader)[0]
            with open(os.path.join(save_dir_path,'Shader_%d.txt' % int(shader)),'w') as f:
                shader=controller.GetShader(pipeline,shader,entry)
                b=shader.rawBytes
                #b=controller.GetShader(shader,entry),rawBytes
                f.write(bytes.decode(b))

def get_draw_call_info_dict(controller,textures_per_dc):
    draw_call_information={}
    for draw_call_description in controller.GetRootActions():
        #define a recursive function for iterating over draws
        def iter_draw(draw_call_description):
            if draw_call_description.GetName(controller.GetStructuredFile()).startswith('glDrawElements') or draw_call_description.GetName(controller.GetStructuredFile()).startswith('glDrawArrays'):
                for api_event in draw_call_description.events:
                    data_chunk=controller.GetStructuredFile().chunks[api_event.chunkIndex]
                    #print(data_chunk.name)
                    if data_chunk.name.startswith('glUseProgram'):
                        stats_info['shaderBindCount']+=1
                    if data_chunk.name.startswith('glBindTexture'):
                        stats_info['textureBindCount']+=1
                
                dic_draw_call={}
                event_id=draw_call_description.eventId
                dic_draw_call["drawIndex"]=draw_call_description.drawIndex
                dic_draw_call["drawcallId"]=draw_call_description.actionId
                dic_draw_call["eventId"]=draw_call_description.eventId
                #dic_draw_call["events"]=draw_call_description.events
                #dic_draw_call["flags"]=draw_call_description.flags
                #dic_draw_call["indexByteWidth"]=draw_call_description.indexByteWidth
                dic_draw_call["name"]=draw_call_description.GetName(controller.GetStructuredFile()).split('(')[0]
                dic_draw_call["numIndices"]=draw_call_description.numIndices
                dic_draw_call["numInstances"]=draw_call_description.numInstances

                dic_draw_call["useTextures"]=[]
                if textures_per_dc.get(event_id):
                    dic_draw_call["useTextures"]=textures_per_dc[event_id]
                draw_call_information[int(event_id)]=dic_draw_call
                dic_draw_call["vertexShader"]=-1
                dic_draw_call["pixelShader"]=-1

                if FetchShaderId:
                    print('replay draw call by event',draw_call_description.eventId)
                    controller.SetFrameEvent(draw_call_description.eventId,True);
                    state=controller.GetPipelineState()
                    #For some APIs,it might be relevant to set the PSO id or entry point name
                    pipe=state.GetGraphicsPipelineObject()
                    
                    vs=state.GetShaderReflection(rd.ShaderStage.Vertex)
                    ps=state.GetShaderReflection(rd.ShaderStage.Fragment)
                    vs_id=int(vs.resourceId) if vs else 0
                    ps_id=int(ps.resourceId) if ps else 0
                    if vs_id and ps_id:
                        program_id=(vs_id,ps_id)
                        if program_id not in stats_info['shaderPrograms']:
                            stats_info['shaderPrograms'].append(program_id)
                    dic_draw_call['vertexShader']=vs_id
                    dic_draw_call['pixelShader']=ps_id

                    #Iterate over the draws children
                    for draw_call_description in draw_call_description.children: 
                        iter_draw(draw_call_description)
     
        iter_draw(draw_call_description)

    return draw_call_information


def save_draw_call_info_csv(resource,save_csv_path):
    count_draw_call,count_indice,count_instance,count_shader_bind,count_texture_bind=0,0,0,0,0
    with open(save_csv_path,'w') as f:
        f.write("Group Name,Event ID,DrawCall Name,Num of Indices," 
                "Num Of Instances,Total Primitive,vertexShader ID,PixelShader ID,use Textures ID\n")
        for id,c in resource.items():
            count_draw_call+=1
            use_textures=",".join([str(t) for t in c["useTextures"]])
            f.write("%s,%s,%s,%s,%s,%s,%s,%s,%s,%s\n" % 
                   (c["groupName"],id,count_draw_call,c["name"],c["numIndices"],c["numInstances"],
                   c["numIndices"]*c["numInstances"]/3,c["vertexShader"],c["pixelShader"],use_textures))
            count_instance+=c["numInstances"]
            count_indice+=c["numInstances"]*c["numIndices"]
    
        count_shader_program=len(stats_info["shaderPrograms"])
    
        f.write(
            "\nDrawCall Count,%d\nTotal Indices,%d\nTotal Instances,%d\nShaderPrograms Used,%d\n"
            "Textures Used,%d\nShaderPrograms Bind,%d\nTextures Bind,%d\n" %
            (count_draw_call,count_indice,count_instance,count_shader_program,
            stats_info['useTextureCount'],stats_info['shaderBindCount'],stats_info['textureBindCount']))

def get_texture_info_dic(controller,events_per_tex):
    texture_information={}
    for texture_description in controller.GetTextures():
      dic_texture_des={}
      texture_id=int(texture_description.resourceId)
      dic_texture_des["arraysize"]=texture_description.arraysize
      dic_texture_des["kbSize"]=round((texture_description.byteSize)/1024,2)
      #print(dic_texture_des["mbSize"])
      dic_texture_des["creationFlags"]=str(texture_description.creationFlags)
      dic_texture_des["cubemap"]=texture_description.cubemap
      dic_texture_des["depth"]=texture_description.depth
      dic_texture_des["dimension"]=texture_description.format.Name()
      dic_texture_des["format"]=texture_description.format.Name()
      dic_texture_des["width"]=texture_description.width
      dic_texture_des["height"]=texture_description.height
      dic_texture_des["msQual"]=texture_description.msQual
      dic_texture_des["msSamp"]=texture_description.msSamp
      dic_texture_des["mips"]=texture_description.mips
      dic_texture_des["type"]=str(texture_description.type).split('.')[-1]
      dic_texture_des["useBy"]=[]
      if events_per_tex.get(texture_id):
          dic_texture_des["useBy"]=events_per_tex[texture_id]
      texture_information[texture_id]=dic_texture_des
    return texture_information

def save_texture_info_csv(resource,save_csv_path):
    with open(save_csv_path,'w') as f:
        f.write("Texture ID,Format,Width,Height,Type,"
                "Mips,kbSize,ArraySize,CreationFlags,Cubemap,"
                "Depth,Dimension,msQual,msSamp\n")
   
        for id,c in resource.items():
             f.write("%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s\n" % 
                    (id,c["format"],c["width"],c["height"],c["type"],
                     c["mips"],c["kbSize"],c["arraysize"],c["creationFlags"],c["cubemap"],
                     c["depth"],
                     c["dimension"],
                     c["msQual"],
                     c["msSamp"]))


def save_texture_info_html(resource,info_dict):
    result="""
    <!DOCTYPE html>
    <html lang="en">
    <head>
        <meta charset="UTF-8">
        <title>Title</title>
    </head>
    <body>
    <div>
       <table border="1" cellpadding="20">
         <tbody align="center">
         <tr>"""

    count_flat=0
    for id,content in resource.items():
        count_flat += 1
    result+="""
               <td width="300">
                 <img src="%s" height="200" width="200">
                 <p>%d,%s</p>
                 <p>Resolution:%d*%d</p>
                 <p>Format:%s</p>
                </td>""" % (content["path"],id,content["name"],info_dict[id]["width"],info_dict[id]["height"],info_dict[id]["format"])
    if not count_flat %6:
                result+="""
         </tr>
         <tr>"""
    result+="""
        </tr>
        </tbody>
        </table>
    </div>
    </body>
    </html>"""
    print(result)

def try_add_call_to_group(drawCall,drawCallGroupList):
    grouped=False

    #Check whether the draw call belongs to a existed group 
    # drawCallGroup
    #events - list of texture resources ids in this group
    #textures - list of texture resources ids in this group
    #vertexShader-vertex shader id of this group 
    #pixelShader - pixel shader id of this group
    # keyTextures - list of  texture ids used to check whether a draw call belongs to this group
    print(drawCallGroupList)
    for groupName,drawCallGroup in drawCallGroupList.items():
        sameShader=drawCall["vertexShader"]==drawCallGroup["vertexShader"] and drawCall["pixelShader"] ==\
                   drawCallGroup["pixelShader"]
        if sameShader:
           print("sameShader")
           similarTextures=False
           if len(drawCallGroup["keyTextures"])>0:
               print("similarTextures is True")
               similarTextures=True;
               for keyTextureId in drawCallGroup["keyTextures"]:
                   if keyTextureId not in drawCall["useTextures"]:
                       similarTextures=False
                       break
            #There must be only one event in this group if the key texture does not exist
           elif len(drawCall["useTextures"])==len(drawCallGroup["textures"]):
               print(drawCall["useTextures"])
               print(drawCallGroup["textures"])
               totalTextureCount=len(drawCall["useTextures"])
               for textureId in drawCall["useTextures"]:
                   if textureId in drawCallGroup["textures"]:
                       drawCallGroup['keyTextures'].append(textureId)
                
               if totalTextureCount<=2:
                   similarTextures=len(drawCallGroup["keyTextures"])==totalTextureCount
               else:
                   similarTextures=len(drawCallGroup["keyTextures"])>(totalTextureCount/2)

               if similarTextures==False:
                  print("Enter AAAAA")
                  drawCallGroup["keyTextures"]=[]
                   
           if similarTextures:
               grouped=True
             
               drawCallGroup["events"].append(int(drawCall["eventId"]))
               texturesToAdd=[texId for texId in drawCall.get("useTextures",[]) if
                             texId not in drawCallGroup["textures"]]
               drawCallGroup["textures"]+=texturesToAdd

               drawCall["groupName"]=groupName
               break
    # if the draw call doesnot belong to any group,create a new group for it
    if grouped==False:
       newGroup={}
       newGroup["events"]=[drawCall["eventId"]]
       newGroup["textures"]=drawCall["useTextures"][:] #deeep copy
       newGroup["vertexShader"]=drawCall["vertexShader"]
       newGroup["pixelShader"]=drawCall["pixelShader"]
       newGroup["keyTextures"]=[]

       groupName="Unknown"+str(len(drawCallGroupList))
       drawCallGroupList[groupName]=newGroup
       drawCall["groupName"]=groupName

def get_draw_call_groups(drawcall_info):
    drawCallGroupList={}
    for eventId,drawCall in drawcall_info.items():
        try_add_call_to_group(drawCall,drawCallGroupList)
    return drawCallGroupList

def print_draw_call_groups_html(drawCallGroupList,save_path):
    contents=""
    for groupName,drawCallGroup in drawCallGroupList.items():
        tex_list,eve_list=drawCallGroup["textures"],drawCallGroup["events"]
        contents+="""
    <article class="layout_table">"""
        contents+=print_draw_call_table(eve_list,tex_list,stats_info['drawcallInfoDic'])
        contents+=print_texture_table(tex_list,saved_texture,stats_info['textureInfoDic'])
        contents+="""
    </article>"""
    with open(save_path,'w') as f:
        f.write("""

        """)


def get_destination_dir(source_file):
    d=os.path.dirname(source_file)
    dir_name=os.path.basename(source_file).split('.')[0]
    dir_name=dir_name.replace('#','')
    des=os.path.join(d,dir_name)
    #des=os.path.join(d,dir_name+time.strftime("-%m.%d-%H.%M,%S"))
    if not os.path.exists(des):
        os.makedirs(des)
    return des,dir_name

def exit_with_data_missing():
    print('Missing stats_info,please reparse %s.rdc.' % filename)
    exit(-1)

def exit_without_rdc():
    print('%s.rdc file not exist!' % filename )
    exit(-1)

def check_stats_info(s):
    if not s.get('eventsPerTexture'):
        exit_with_data_missing()
    elif not s.get('texturesPerDrawcall'):
        exit_with_data_missing()
    elif not s.get('savedTextures'):
        exit_with_data_missing()

stats_info=dict()
stats_info['shaderBindCount']=0
stats_info['textureBindCount']=0
stats_info['useTextureCount']=0
stats_info['drawcallInfoDict']={}
stats_info['textureInfoDic']={}
stats_info['shaderPrograms']=[]

des_dir,filename=get_destination_dir(rdc_file_path)

if not os.path.exists(rdc_file_path):
    exit_without_rdc()

saved_data=os.path.join(des_dir,filename+'.pkl')

if os.path.exists(saved_data) and not NeedReparse:
    f=open(saved_data,'rb')
    try:
        stats_info=pickle.load(f)
        f.close()
        check_stats_info(stats_info)
    except EOFError:
        exit_with_data_missing()
else:
    cap,controller=loadCapture(rdc_file_path)
    #1:Save Shader.txt
    save_shaders(controller,os.path.join(des_dir,'shaders'))
    #2:Save Texture.png
    stats_info['savedTextures']=save_textures(controller,os.path.join(des_dir,'textures'))
    
    #3:Get list of textureid
    stats_info['eventsPerTexture'],stats_info['texturesPerDrawcall']=get_texture_usage(controller)
    stats_info['drawcallInfoDic']=get_draw_call_info_dict(controller,stats_info['texturesPerDrawcall'])
    stats_info['textureInfoDic']=get_texture_info_dic(controller,stats_info['eventsPerTexture'])

    f=open(saved_data,'wb')
    pickle.dump(stats_info,f,0)
    f.close()
    
    controller.Shutdown()
    cap.Shutdown()

if __name__ == '__main__':
    events_per_tex=stats_info['eventsPerTexture']
    textures_per_dc=stats_info['texturesPerDrawcall']
    saved_texture=stats_info['savedTextures']

    drawCallGroupList=get_draw_call_groups(stats_info['drawcallInfoDic'])

    save_draw_call_info_csv(
        stats_info['drawcallInfoDic'],
        os.path.join(des_dir,'drawCall_%s.csv' % filename))
    save_texture_info_csv(
        stats_info['textureInfoDic'],
         os.path.join(des_dir,'texture_%s.csv' % filename))
    
    #print_draw_call_groups_html(drawCallGroupList,os.path.join(des_dir,'groupInfo_%s.html' % filename))
    
    #print_custom_table(ycity,
    #) os.path.join(des_dir,'customGroupInfo_%s.html' % filename)
    
    
  
    
        