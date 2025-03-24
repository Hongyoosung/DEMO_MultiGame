import unreal
import os
import sys
import csv
import re

# 설정
project_name = "DEMO_MultiGame"
csv_folder = unreal.SystemLibrary.get_project_directory() + "CSV"
struct_save_folder = unreal.SystemLibrary.get_project_directory() + "Source/" + project_name + "/Tables/"



# 타입 매핑 딕셔너리
type_mapping = {
    "int": "int32",
    "float": "float",
    "string": "FString",
    "bool": "bool",
    "vector": "FVector",
    "rotator": "FRotator",
    "text": "FText",
    "color": "FLinearColor"
}



# 타입 변환 함수
def get_unreal_type(type_string):
    type_string = type_string.lower()
    
    for key, value in type_mapping.items():
        if re.match(key, type_string):
            return value
            
    unreal.log_warning(f"Unsupported type: {type_string}. Defaulting to FString.")
    return "FString"



# CSV 파일 처리 함수
def read_csv_file(file_path):
    try:
        with open(file_path, 'r', encoding='utf-8') as csvfile:
            reader = csv.reader(csvfile)
            return list(reader)
    except Exception as e:
        unreal.log_error(f"Error reading CSV file {file_path}: {e}")
        return None



def find_column_name_row_index(rows):
    for i, row in enumerate(rows):
        if row and (row[0].lower() == "id"):
            return i
            
    return -1



def extract_column_names_and_types(rows, column_name_row_index):
    column_names = []
    type_names = []

    if column_name_row_index <= 0:
        unreal.log_error("Invalid column name row index.")
        return [], []

    type_row = rows[column_name_row_index - 1]
    column_name_row = rows[column_name_row_index]

    for i, column_name in enumerate(column_name_row):
        if column_name.startswith("#"):
            continue
            
        column_names.append(column_name)
        
        if i < len(type_row):
            type_names.append(type_row[i])
        else:
            unreal.log_warning(f"No type specified for column {column_name}. Defaulting to FString.")
            type_names.append("string")  # 기본 타입 설정

    return column_names, type_names
    
    

# C++ 구조체 생성 함수
def generate_struct_header(file_name):
    header = f"""// Copyright YourName, MIT LICENSE
// This file is auto-generated.
#pragma once
#include "Engine/DataTable.h"
#include "{file_name}.generated.h"

USTRUCT(BlueprintType)
struct F{file_name} : public FTableRowBase
{{
    GENERATED_BODY()

public:
"""
    return header



def generate_struct_body(column_names, type_names):
    body = ""
    
    for column_name, type_name in zip(column_names, type_names):
        if column_name.lower() == "id":
            continue
        unreal_type = get_unreal_type(type_name)
        body += f"\tUPROPERTY(EditAnywhere, BlueprintReadWrite)\n"
        body += f"\t{unreal_type} {column_name};\n"
        
    return body



def write_struct_file(struct_save_folder, file_name, content):
    file_path = os.path.join(struct_save_folder, f"F{file_name}.h")
    
    try:
        os.makedirs(struct_save_folder, exist_ok=True)
        with open(file_path, 'w') as file:
            file.write(content)
        return True
        
    except Exception as e:
        unreal.log_error(f"Error writing to file {file_path}: {e}")
        return False



# 메인 함수
def create_struct():
    print("####### Data Table C++ Struct Generator Started! #######")
    print("###### Target CSV Folder : " + csv_folder)

    csv_files = [f for f in os.listdir(csv_folder) if f.endswith('.csv')]
    if not csv_files:
        unreal.log_error("No CSV files found in the specified folder.")
        return

    for csv_file in csv_files:
        print(f"Processing {csv_file}...")
        
        file_path = os.path.join(csv_folder, csv_file)
        rows = read_csv_file(file_path)
        
        if not rows:
            continue

        column_name_row_index = find_column_name_row_index(rows)
        if column_name_row_index == -1:
            unreal.log_error(f"Could not find 'Id' column in {csv_file}. Skipping file.")
            continue

        column_names, type_names = extract_column_names_and_types(rows, column_name_row_index)
        if not column_names or not type_names:
            unreal.log_error(f"Could not extract column names or types from {csv_file}. Skipping file.")
            continue

        if len(column_names) != len(type_names):
            unreal.log_error(f"Number of columns and types do not match in {csv_file}. Skipping file.")
            continue


        file_name = os.path.splitext(csv_file)[0]
        header = generate_struct_header(file_name)
        body = generate_struct_body(column_names, type_names)
        content = header + body + "};\n"


        if write_struct_file(struct_save_folder, file_name, content):
            print(f"Successfully generated struct for {csv_file}")
        else:
            print(f"Failed to generate struct for {csv_file}")



# 실행
create_struct()
print("********* C++ Struct Generator Closed. **********")
