## 模型描述文件
1. 模型描述文件
    - 文件格式后缀名： .mud
    - 文件内容使用 INI 格式，内容定义如下：

        [basic]

        type =  awnn / aipu

        param = example.param

        bin = example.bin

        [inputs]

        input0 = h , w , c , mean_R , mean_G ，mean_B ，norm_R ，norm_G ，norm_B

        input1 = h , w , c , mean_R , mean_G ，mean_B ，norm_R ，norm_G ，norm_B

        input2 = h , w , c , mean_R , mean_G ，mean_B ，norm_R ，norm_G ，norm_B

        ……

        [outputs]

        output0 = h,w,c

        output1 = h,w,c

        output2 = h,w,c

        output0 = h,w,c

        ……

        [extra]

        inputs_scale = input0_scale , input1_scale, input2_scale ……

        outputs_scale = ouput0_scale , output1_scale , output2_scale ……



        [decoder]

        name = Retinaface

        steps = 8 ，16，32，64

        min_sizes = 10, 16, 24, 32, 48, 64, 96, 128, 192, 256

        variance = 0.1 ， 0.2

        label = "cat" , "dog" ……


2. 描述文件关键词说明
不同目标平台的标识， 如R329 称为aipu， V831代称为awnn。

[basic]
路径参数段， 其中的字段定义模型源文件
- param（required）

    V831模型组成之一， R329不会用到此值，但是可以置为空
- bin

    V831 , R329 平台模型的重要组成

[inputs]
解析节选， 其中定义输入个数和相关参数
- Input
输入节点名称，前三个参数定义为  H,W,C ，后面的参数以此是mean，norm。
特此说明:
    - H != 1 && W != 1 && C == 3 的时候，输入为三通道图像，依次按照mean_R , mean_G,mean_B ，norm_R ，norm_G ，norm_B  的顺序往后排列
    - H != 1 && W != 1 && C == 1 的时候，输入为灰度图， mean_gray , norm_gray ，按顺序两个值，进行排列。
    - H ==  1 && W == 1 && C != 1 ，输入为三维向量， mean_v , norm_v ，按顺序两个值，进行排列。

Input0,input1, input2 ……称为模型输入节点的名字，等号后的信息是对应输入节点的相关信息。具体多少个输入需根据模型确定。

[outputs]
输出信息段， 其中的字段定义输出个数和相关参数
- output 输出节点名称， 其值按照 h , w ,c  排列，输出节点个数随模型确定

[extra]
额外的参数段，一般是不同平台额外的参数。比如 AIPU 由于原 AIPU 模型文件内无法获取到量化的 scale 信息，而且参数和模型唯一绑定，可以在这里填写供底层查询
- inputs_scale
输入层在量化后的scale值，按照输入顺序排列 ，如inputs_scale = input0_scale , input1_scale, input2_scale ……
- outputs_scale
输出层在量化后的scale值，按照输出顺序排列，如outputs_scale = ouput0_scale , output1_scale , output2_scale ……

[decoder]
解析参数段， 其中定义后处理的信息
- name:
后处理解码器选择，代码可以根据这个名字自动选择解码器，从而将后续的参数自动读入
- steps:
retinaface 特有：特征金字塔缩小倍数
- min_sizes:
retinaface 特有：锚框
- variance:
retinaface 特有：
- label:
标签


1. .mdsc文件的使用

```python
    m = nn.load("*.mud")
    outs = m.forward(inputs)
    decoder = nn.get_decoder("*.mud")
    print(decoder.id)
    outs = decoder.run(outs)
```

R329 example:
``` cfg
[basic]
type = aipu
param =
bin =/root/models/aipu_Retinaface_320.bin

[inputs]
input0 = 320,320,3,104,117,123,1,1,1

[outputs]
output0 = 1,4,5875
output1 = 1,2,5875
output2 = 1,10,5875

[extra]
outputs_scale =32.752407 , 29.865177 , 14.620169

```
v831 example  (in maintenance)
``` cfg
[basic]
type = awnn
param =/root/models/awnn_retinaface.parma
bin =/root/models/awnn_retinaface.bin

[inputs]
input0 = 224,224,3,127.5, 127.5, 127.5,0.0078125, 0.0078125, 0.0078125

[outputs]
output0 = 1,4,2058
output1 = 1,2,2058
output2 = 1,10,2058

[extra]
outputs_scale =
inputs_scale=

```