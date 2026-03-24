# talk

| 参数名称 | 参数类型 | 默认值 | 备注 |
| ------- | -------- | ----- | ---- |
| \_\_charname\_\_ | String | （空） | |
| \_\_text\_\_ | String | （空） | |
| \_\_isaddition\_\_ | Bool | False | |
| \_\_nochangeslot\_\_ | Bool | False | |
| \_\_staytime\_\_ | Number | 1.75 |范围：0 ~ +∞（端点可取）|
| \_\_interval\_\_ | Number | 0.04 |范围：0 ~ +∞（端点可取）|
# bg

| 参数名称 | 参数类型 | 默认值 | 备注 |
| ------- | -------- | ----- | ---- |
| \_\_bgname\_\_ | String | 无 | |
| \_\_fadeintime\_\_ | Number | 2.0 |范围：0 ~ +∞（端点可取）|
| \_\_startpos\_\_ | Vector | [0, 0, 10] | |
| \_\_endpos\_\_ | Vector | [0, 0, 10] | |
| \_\_movetime\_\_ | Number | 2.0 |范围：0 ~ +∞（端点可取）|
| \_\_movecurve\_\_ | Enum | Linear |枚举值：Linear, In, Out, InOut, In2, Out2, InOut2, In3, Out3, InOut3, Bounce, ToFrom, Shake|
# music

| 参数名称 | 参数类型 | 默认值 | 备注 |
| ------- | -------- | ----- | ---- |
| \_\_msname\_\_ | String | （空） | |
| \_\_introname\_\_ | String | （空） | |
| \_\_volume\_\_ | Number | 1.0 |范围：0.0 ~ 1.0（端点可取）|
| \_\_slot\_\_ | Number | 无 |范围：0 ~ 1（端点可取）**此数字参数应该为整数**|
| \_\_changetime\_\_ | Number | 1.5 |范围：0 ~ +∞（端点可取）|
# sound

| 参数名称 | 参数类型 | 默认值 | 备注 |
| ------- | -------- | ----- | ---- |
| \_\_sdname\_\_ | String | （空） | |
| \_\_volume\_\_ | Number | 1 |范围：0.0 ~ 1.0（端点可取）|
| \_\_iscv\_\_ | Bool | False | |
# cover

| 参数名称 | 参数类型 | 默认值 | 备注 |
| ------- | -------- | ----- | ---- |
| \_\_cover\_\_ | Bool | False | |
# avg

| 参数名称 | 参数类型 | 默认值 | 备注 |
| ------- | -------- | ----- | ---- |
| \_\_pcname\_\_ | String | （空） | |
| \_\_slot\_\_ | Number | 无 |范围：0 ~ +∞（端点可取）**此数字参数应该为整数**|
| \_\_diff\_\_ | String | （空） | |
| \_\_ishide\_\_ | Number | 0 |范围：0.0 ~ 3.0（左端点可取）（右端点不可取）|
| \_\_fadeintime\_\_ | Number | 0 |范围：0 ~ +∞（端点可取）|
| \_\_fadeouttime\_\_ | Number | 0 |范围：0 ~ +∞（端点可取）|
| \_\_isblock\_\_ | Bool | True | |
| \_\_startpos\_\_ | Vector | ['x', 0, 7] | |
| \_\_endpos\_\_ | Vector | ['x', 0, 7] | |
| \_\_movetime\_\_ | Number | 0 |范围：0 ~ +∞（端点可取）|
| \_\_movecurve\_\_ | Enum | Out |枚举值：Linear, In, Out, InOut, In2, Out2, InOut2, In3, Out3, InOut3, Bounce, ToFrom, Shake|
# flash

| 参数名称 | 参数类型 | 默认值 | 备注 |
| ------- | -------- | ----- | ---- |
| \_\_color\_\_ | Vector | [1, 1, 1, 1] |范围：0 ~ 1（端点可取）|
| \_\_fadeintime\_\_ | Number | 0.1 |范围：0 ~ +∞（端点可取）|
| \_\_staytime\_\_ | Number | 0 |范围：0 ~ +∞（端点可取）|
| \_\_fadeouttime\_\_ | Number | 0.5 |范围：0 ~ +∞（端点可取）|
| \_\_isblock\_\_ | Bool | True | |
# effect

| 参数名称 | 参数类型 | 默认值 | 备注 |
| ------- | -------- | ----- | ---- |
| \_\_name\_\_ | String | （空） |可选值：右日光, 多利, 左日光, 橙火, 紫火, 红尘, 聚光灯, 蒸汽, 血潮, 辩论, 雨, 雨滴, 雪, 暴风, 冰法术, 刀光1, 刀光2, 刀光击中1, 刀光击中2, 刀光火花1, 刀光火花2, 水显, 水遁, 法术击中, 火法术, 火爆, 电法术, 落沙, 钝击1, 钝击2, 长枪刺击, 长枪命中, 闪光击中, 信号雪花1, 信号雪花2, 黑幕上, 黑幕下, 黑幕左, 黑幕右, 表情_不悦1, 表情_不悦2, 表情_介意, 表情_害羞, 表情_开心1, 表情_开心2, 表情_流汗, 表情_疑惑, 表情_骄傲|
| \_\_lifestep\_\_ | Number | 1 |范围：1 ~ +∞（端点可取）**此数字参数应该为整数**|
| \_\_pos\_\_ | Vector | [0, 0, 0] | |
| \_\_simulationspeed\_\_ | Number | 1 |范围：0 ~ +∞（端点可取）|
# shake

| 参数名称 | 参数类型 | 默认值 | 备注 |
| ------- | -------- | ----- | ---- |
| \_\_force\_\_ | Vector | [0.5, 0.5, 0] | |
| \_\_fadeintime\_\_ | Number | 0.1 |范围：0 ~ +∞（端点可取）|
| \_\_staytime\_\_ | Number | 0.5 |范围：0 ~ +∞（端点可取）|
| \_\_fadeouttime\_\_ | Number | 0.5 |范围：0 ~ +∞（端点可取）|
| \_\_isblock\_\_ | Bool | True | |
# filter

| 参数名称 | 参数类型 | 默认值 | 备注 |
| ------- | -------- | ----- | ---- |
| \_\_filterid\_\_ | Number | 0 |范围：0 ~ 4（端点可取）**此数字参数应该为整数**|
| \_\_force\_\_ | Number | 1 |范围：0 ~ +∞（端点可取）|
| \_\_fadeintime\_\_ | Number | 2 |范围：0 ~ +∞（端点可取）|
# delay

| 参数名称 | 参数类型 | 默认值 | 备注 |
| ------- | -------- | ----- | ---- |
| \_\_delaytime\_\_ | Number | 0 |范围：0 ~ +∞（端点可取）|
# option

| 参数名称 | 参数类型 | 默认值 | 备注 |
| ------- | -------- | ----- | ---- |
| \_\_optionX\_\_ | String | （空） | |
# jump

| 参数名称 | 参数类型 | 默认值 | 备注 |
| ------- | -------- | ----- | ---- |
| \_\_sign\_\_ | String | （空） | |
| \_\_customparams\_\_ | String | （空） | |
# hologram

| 参数名称 | 参数类型 | 默认值 | 备注 |
| ------- | -------- | ----- | ---- |
| \_\_text\_\_ | String | （空） | |
| \_\_showall\_\_ | Bool | False | |
| \_\_isItalic\_\_ | Bool | False | |
| \_\_fadeintime\_\_ | Number | 0 |范围：0 ~ +∞（端点可取）|
| \_\_fadeouttime\_\_ | Number | 0 |范围：0 ~ +∞（端点可取）|
| \_\_staytime\_\_ | Number | 1.75 |范围：0 ~ +∞（端点可取）|
| \_\_pos\_\_ | Vector | [0, 0] | |
| \_\_isaddition\_\_ | Bool | False | |
| \_\_slot\_\_ | Number | 0 |范围：0 ~ +∞（端点可取）**此数字参数应该为整数**|
| \_\_interval\_\_ | Number | 0.04 |范围：0 ~ +∞（端点可取）|
# tele

| 参数名称 | 参数类型 | 默认值 | 备注 |
| ------- | -------- | ----- | ---- |
| \_\_ison\_\_ | Bool | False | |
| \_\_pcname\_\_ | String | （空） | |
| \_\_diff\_\_ | String | （空） | |
| \_\_ishide\_\_ | Bool | False | |
| \_\_pos\_\_ | Vector | [0, 0, 7] | |
| \_\_slot\_\_ | Number | 0 |范围：0 ~ +∞（端点可取）**此数字参数应该为整数**|
| \_\_style\_\_ | Number | 0 |范围：0 ~ 1（端点可取）**此数字参数应该为整数**|
