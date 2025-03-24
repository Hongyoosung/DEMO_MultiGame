import unreal
import os
import sys
import csv
import re



# 설정
project_name = "DEMO_MultiGame"
asset_class = unreal.DataTable
asset_path = "/Data"
csv_folder = unreal.SystemLibrary.get_project_directory() + "CSV"



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



# "Id" 컬럼을 찾는 함수
def find_id_row_index(rows):
    for index, row in enumerate(rows):
        if row and str(row[0]).lower() == "id":
            return index
            
    return -1



# 임시 CSV 파일을 생성하는 함수
def create_temp_csv_file(rows, id_row_index, temp_csv_path):
    try:
        with open(temp_csv_path, 'w', newline='') as temp_csv:
            writer = csv.writer(temp_csv)
            
            for index, row in enumerate(rows):
                if index >= id_row_index:
                    writer.writerow(row)
                    
        return True
    except Exception as e:
        unreal.log_error(f"Error creating temporary CSV file {temp_csv_path}: {e}")
        return False



# 에셋 임포트 함수
def import_asset(task):
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])



# 데이터 테이블 에셋 생성 함수
def create_data_table_asset(csv_path, struct_path, asset_path):
    file_name = os.path.splitext(os.path.basename(csv_path))[0]
    asset_name = "DT_" + file_name
    temp_csv_path = os.path.join(unreal.SystemLibrary.get_project_directory(), "Temp_" + asset_name + ".csv")

    unreal_struct_path = "/Script/" + project_name + "." + file_name


    print(f"--------- Creating data table asset: {asset_name} ----------")
    print(f"  CSV path: {csv_path}")
    print(f"  Struct path: {unreal_struct_path}")
    print(f"  Asset path: {asset_path}")


    try:
        # 구조체 로드
        struct_object = unreal.load_object(None, unreal_struct_path)
        if not struct_object:
            unreal.log_error(f"Failed to load struct: {unreal_struct_path}")
            return

        # CSV 파일 읽기
        rows = read_csv_file(csv_path)
        if not rows:
            return

        # "Id" 컬럼 찾기
        id_row_index = find_id_row_index(rows)
        if id_row_index == -1:
            unreal.log_error("Cannot find 'Id' column.")
            return

        # 임시 CSV 파일 생성
        if not create_temp_csv_file(rows, id_row_index, temp_csv_path):
            return

        # CSV 임포트 팩토리 설정
        csv_factory = unreal.CSVImportFactory()
        csv_factory.automated_import_settings.import_row_struct = struct_object

        # 에셋 임포트 Task 생성 및 설정
        task = unreal.AssetImportTask()
        task.filename = temp_csv_path
        task.destination_name = asset_name
        task.destination_path = asset_path
        task.replace_existing = True
        task.automated = True
        task.save = True
        task.factory = csv_factory

        # 에셋 임포트
        import_asset(task)
        print(f"  Successfully imported asset: {asset_name}")


    except Exception as e:
        unreal.log_error(f"Error creating data table asset {asset_name}: {e}")
    finally:
        # 임시 파일 삭제 (try-finally로 삭제 보장)
        try:
            os.remove(temp_csv_path)
            print(f"  Successfully removed temporary file: {temp_csv_path}")
        except FileNotFoundError:
            unreal.log_warning(f"Temporary file not found: {temp_csv_path}")
        except Exception as e:
            unreal.log_error(f"Error removing temporary file {temp_csv_path}: {e}")



# 시작 함수
def start():
    print("####### Data Table Asset Generator Started! #######")
    print("###### Target CSV Folder : " + csv_folder)

    # CSV 파일 목록 가져오기
    csv_files = [f for f in os.listdir(csv_folder) if f.endswith(".csv")]
    if not csv_files:
        unreal.log_error("There's no CSV file in folder : " + csv_folder)
        return

    print("----------- CSV File List ------------")
    for index, file in enumerate(csv_files):
        print(f"({index + 1}) {file}")

    # CSV 파일 순회하며 데이터 테이블 에셋 생성
    for file in csv_files:
        print("--------------------------------------------------")
        print(f"::::::::::::: Start making [{file}] ::::::::::::::")
        csv_file_path = os.path.join(csv_folder, file)
        create_data_table_asset(csv_file_path, asset_path, asset_path)



# 실행
start()
print("********* Asset Generator Closed. **********")
