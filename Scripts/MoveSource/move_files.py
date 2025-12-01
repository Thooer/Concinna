#!/usr/bin/env python3
"""
文件移动脚本
根据MoveConfig.yaml配置，将指定文件夹中的特定后缀文件复制到时间戳命名的输出目录中
"""

import os
import shutil
import yaml
from datetime import datetime
from pathlib import Path


def load_configs(base_dir):
    """加载配置文件"""
    extensions_path = os.path.join(base_dir, "Extensions.yaml")
    folders_path = os.path.join(base_dir, "Folders.yaml")
    
    # 加载文件后缀配置 (直接是数组)
    with open(extensions_path, 'r', encoding='utf-8') as file:
        extensions = yaml.safe_load(file)
    
    # 加载文件夹配置 (直接是字典)
    with open(folders_path, 'r', encoding='utf-8') as file:
        folders_config = yaml.safe_load(file)
    
    return extensions, folders_config


def find_files_with_extensions(base_dir, extensions, target_folders):
    """递归查找指定文件夹中特定后缀的文件"""
    found_files = []
    
    # 直接在项目根目录查找
    project_root = os.path.dirname(os.path.dirname(base_dir))
    
    def process_subfolders(folder_path, folder_config, parent_name=""):
        """递归处理子文件夹配置"""
        if isinstance(folder_config, dict):
            # 如果是字典，说明有嵌套结构
            for subfolder_name, subfolder_config in folder_config.items():
                subfolder_path = os.path.join(folder_path, subfolder_name)
                current_parent_name = f"{parent_name}/{subfolder_name}" if parent_name else subfolder_name
                
                if os.path.exists(subfolder_path):
                    if isinstance(subfolder_config, list):
                        # 如果是列表，直接搜索每个指定的子文件夹
                        for item in subfolder_config:
                            if isinstance(item, str):
                                # 字符串类型，直接搜索
                                item_path = os.path.join(subfolder_path, item)
                                if os.path.exists(item_path):
                                    print(f"  搜索子文件夹: {item_path}")
                                    search_and_add_files(item_path, current_parent_name, item)
                            elif isinstance(item, dict):
                                # 字典类型，递归处理嵌套结构
                                process_subfolders(subfolder_path, {item_name: item_config for item_name, item_config in item.items()}, current_parent_name)
                    else:
                        # 非列表类型，直接搜索该文件夹
                        print(f"  搜索子文件夹: {subfolder_path}")
                        search_and_add_files(subfolder_path, current_parent_name, subfolder_name)
        elif isinstance(folder_config, list):
            # 如果直接是列表，搜索每个项目
            for item in folder_config:
                if isinstance(item, str):
                    item_path = os.path.join(folder_path, item)
                    if os.path.exists(item_path):
                        print(f"  搜索子文件夹: {item_path}")
                        search_and_add_files(item_path, parent_name, item)
                elif isinstance(item, dict):
                    # 字典类型，递归处理
                    process_subfolders(folder_path, item, parent_name)
        else:
            # 如果不是字典或列表，或配置为空，直接搜索整个文件夹
            if os.path.exists(folder_path) and folder_config is not None:
                print(f"  搜索子文件夹: {folder_path}")
                search_and_add_files(folder_path, parent_name, os.path.basename(folder_path))
    
    def search_and_add_files(search_path, folder_name, relative_path):
        """搜索并添加文件"""
        for root, dirs, files in os.walk(search_path):
            for file in files:
                # 检查文件后缀
                if any(file.endswith(ext) for ext in extensions):
                    full_path = os.path.join(root, file)
                    # 计算相对于搜索路径的路径
                    file_relative_path = os.path.relpath(full_path, search_path)
                    found_files.append((full_path, folder_name, relative_path, file_relative_path))
    
    # 处理层级结构配置
    for folder_name, folder_config in target_folders.items():
        folder_path = os.path.join(project_root, folder_name)
        
        if not os.path.exists(folder_path):
            print(f"警告: 文件夹 {folder_path} 不存在，跳过")
            continue
            
        print(f"搜索文件夹: {folder_path}")
        
        if folder_config:
            process_subfolders(folder_path, folder_config, folder_name)
        else:
            # 如果配置为空或None，搜索整个文件夹
            search_and_add_files(folder_path, folder_name, folder_name)
    
    return found_files


def create_timestamp_dir(output_base_dir):
    """创建以时间戳命名的输出目录"""
    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    output_dir = os.path.join(output_base_dir, timestamp)
    
    # 创建目录
    os.makedirs(output_dir, exist_ok=True)
    print(f"创建输出目录: {output_dir}")
    
    return output_dir


def copy_and_rename_file(source_path, folder_name, relative_path, file_relative_path, output_dir):
    """复制文件并重命名（扁平结构，文件名包含完整路径信息）"""
    
    # 清理folder_name中的路径分隔符和非法字符
    clean_folder_name = folder_name.replace(os.sep, '.').replace('/', '.').replace('\\', '.')
    clean_folder_name = clean_folder_name.replace(':', '_').replace('*', '_').replace('?', '_').replace('"', '_').replace('<', '_').replace('>', '_').replace('|', '_')
    
    # 构建完整路径：folder_name + relative_path + file_relative_path
    # 首先清理relative_path
    clean_relative_path = relative_path.replace(os.sep, '.').replace('/', '.').replace('\\', '.')
    clean_relative_path = clean_relative_path.replace(':', '_').replace('*', '_').replace('?', '_').replace('"', '_').replace('<', '_').replace('>', '_').replace('|', '_')
    
    # 清理file_relative_path
    clean_file_relative_path = file_relative_path.replace(os.sep, '.').replace('/', '.').replace('\\', '.')
    clean_file_relative_path = clean_file_relative_path.replace(':', '_').replace('*', '_').replace('?', '_').replace('"', '_').replace('<', '_').replace('>', '_').replace('|', '_')
    
    # 组合完整的路径信息
    full_path_components = [clean_folder_name]
    if clean_relative_path and clean_relative_path != clean_folder_name:
        full_path_components.append(clean_relative_path)
    full_path_components.append(clean_file_relative_path)
    
    # 构建新的文件名
    new_filename = '.'.join(full_path_components)
    
    # 构建目标路径 - 扁平化结构，所有文件都在输出目录根下
    target_path = os.path.join(output_dir, new_filename)
    
    try:
        # 复制文件
        shutil.copy2(source_path, target_path)
        print(f"复制: {source_path} -> {target_path}")
        return True
    except Exception as e:
        print(f"错误: 无法复制文件 {source_path}: {e}")
        print(f"  目标文件名: {new_filename}")
        print(f"  目标路径: {target_path}")
        return False


def merge_copied_files(output_dir):
    merged_file_path = os.path.join(output_dir, "Amerge.txt")
    files_to_merge = []
    for root, dirs, files in os.walk(output_dir):
        for f in files:
            if f == "Amerge.txt":
                continue
            files_to_merge.append(os.path.join(root, f))
    files_to_merge.sort()
    try:
        with open(merged_file_path, 'w', encoding='utf-8') as out:
            for filepath in files_to_merge:
                name = os.path.relpath(filepath, output_dir)
                out.write(f"{name}:\n")
                with open(filepath, 'r', encoding='utf-8', errors='replace') as fin:
                    out.write(fin.read())
                out.write("\n")
        print(f"生成合并文件: {merged_file_path}")
    except Exception as e:
        print(f"错误: 无法生成合并文件: {e}")
    return len(files_to_merge)


def main():
    """主函数"""
    # 设置路径
    script_dir = os.path.dirname(os.path.abspath(__file__))
    output_dir_base = os.path.join(script_dir, "Out")
    
    # 加载配置
    try:
        extensions, target_folders = load_configs(script_dir)
        
        print(f"配置文件加载成功")
        print(f"目标后缀: {extensions}")
        print(f"目标文件夹: {target_folders}")
        
    except Exception as e:
        print(f"错误: 无法加载配置文件: {e}")
        return
    
    # 创建输出目录
    output_dir = create_timestamp_dir(output_dir_base)
    
    # 查找文件
    print("\n开始搜索文件...")
    found_files = find_files_with_extensions(script_dir, extensions, target_folders)
    
    if not found_files:
        print("未找到符合条件的文件")
        return
    
    print(f"找到 {len(found_files)} 个文件")
    
    # 复制并重命名文件
    print("\n开始复制文件...")
    success_count = 0
    for source_path, folder_name, relative_path, file_relative_path in found_files:
        if copy_and_rename_file(source_path, folder_name, relative_path, file_relative_path, output_dir):
            success_count += 1
    
    print("\n开始合并文件...")
    merged_count = merge_copied_files(output_dir)
    print(f"合并完成: {merged_count} 个文件")
    print(f"合并文件: {os.path.join(output_dir, 'Amerge.txt')}")

    # 输出结果
    print(f"\n操作完成!")
    print(f"成功复制: {success_count} 个文件")
    print(f"输出目录: {output_dir}")
    print(f"失败: {len(found_files) - success_count} 个文件")


if __name__ == "__main__":
    main()
