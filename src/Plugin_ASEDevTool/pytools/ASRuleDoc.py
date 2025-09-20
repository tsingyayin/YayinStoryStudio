import json
from typing import *
import sys
import os
def readRuleMeta(filePath:str)->dict:
	ruleMeta = {}
	with open(filePath, 'r', encoding='utf-8') as file:
		try:
			ruleMeta = json.load(file)
		except json.JSONDecodeError as e:
			print(f"Error decoding JSON from {filePath}: {e}")
	return ruleMeta

def isNumber(value:Any)->bool:
	return isinstance(value, (int, float))

def parseRuleMeta(ruleMeta:dict)->str:
	md5Str:str = ""
	for controller in ruleMeta["RuleMeta"]:
		controllerName = controller["Name"]
		md5Str += f"# {controllerName}\n\n"
		params = controller["ParaMeta"].keys()
		md5Str += "| 参数名称 | 参数类型 | 默认值 | 备注 |\n"
		md5Str += "| ------- | -------- | ----- | ---- |\n"

		for param in params:
			paramMeta = controller["ParaMeta"][param]
			paramType = paramMeta['Type']
			md5Str += "| \\_\\_" + param + "\\_\\_ "
			md5Str += f"| {paramType} "
			if "Default" in paramMeta:
				if paramMeta['Default'] == "":
					md5Str += "| （空） "
				else:
					md5Str += f"| {paramMeta['Default']} "
			else:
				md5Str += "| 无 "

			if "Range" in paramMeta:
				rangeMeta:list = paramMeta['Range']
				if len(rangeMeta) == 2:
					if (isNumber(rangeMeta[0]) and isNumber(rangeMeta[1])):
						md5Str += f"|范围：{rangeMeta[0]} ~ {rangeMeta[1]}（端点可取）"
					elif (isNumber(rangeMeta[0]) and type(rangeMeta[1]) is str):
						if "+" in rangeMeta[1]:
							md5Str += f"|范围：{rangeMeta[0]} ~ +∞"
							if "L" in rangeMeta[1]:
								md5Str += f"（端点不可取）"
							else:
								md5Str += f"（端点可取）"
						elif "-" in rangeMeta[1]:
							md5Str += f"范围：-∞ ~ {rangeMeta[1]}"
							if "R" in rangeMeta[1]:
								md5Str += f"（端点不可取）"
							else:
								md5Str += f"（端点可取）"
				elif len(rangeMeta) == 3:
					md5Str += f"|范围：{rangeMeta[0]} ~ {rangeMeta[1]}"
					if "L" in rangeMeta[2]:
						md5Str += f"（左端点不可取）"
					else:
						md5Str += f"（左端点可取）"
					if "R" in rangeMeta[2]:
						md5Str += f"（右端点不可取）"
					else:
						md5Str += f"（右端点可取）"
				if "Integer" in paramMeta and paramMeta['Integer'] is True:
					md5Str += "**此数字参数应该为整数**|\n"
				else:
					md5Str += "|\n"
			elif "Enum" in paramMeta:
				if paramMeta['Type'] == "Enum":
					md5Str += f"|枚举值：{', '.join(paramMeta['Enum'])}|\n"
				elif paramMeta['Type'] == "String":
					md5Str += f"|可选值：{', '.join(paramMeta['Enum'])}|\n"
			else:
				md5Str += "| |\n"
	return md5Str

if __name__ == "__main__":
	# Example usage
	print(os.getcwd())
	path = "Plugin_ASEDevTool/compiled/ASEDevTool/template/2.05.22.1A/RuleMeta.json"
	savePath = "Plugin_ASEDevTool/compiled/ASEDevTool/template/2.05.22.1A/RuleMeta.md"
	ruleMeta = readRuleMeta(path)
	if ruleMeta:
		md5Str = parseRuleMeta(ruleMeta)
		print(md5Str)
		with open(savePath, 'w', encoding='utf-8') as saveFile:
			saveFile.write(md5Str)
	else:
		print("Failed to read rule meta data.")
