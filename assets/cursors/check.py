#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Windows 10 批处理脚本 - 光标主题资源校验器
功能：遍历当前目录下的一级子文件夹，检查：
  1. logo.png 是否存在
  2. config.json 是否存在且格式合法
  3. config.json 中的 "name" 是否与文件夹名一致
  4. "cursor" 对象中每个非空文件名是否在文件夹内存在
  5. "author" 和 "source" 是否为空（应非空）
"""

import os
import json
import sys

# 设置控制台输出为 UTF-8（避免中文乱码）
if sys.stdout.encoding != 'utf-8':
    try:
        sys.stdout.reconfigure(encoding='utf-8')
    except AttributeError:
        import io
        sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8')


def check_directory(dir_path, dir_name):
    """
    对单个子文件夹执行所有校验，返回结果字典。
    """
    result = {
        'dir': dir_name,
        'logo_exists': False,
        'config_exists': False,
        'name_match': False,
        'cursor_files_exist': True,   # 默认真，遇缺失则改假
        'author_empty': False,
        'source_empty': False,
        'errors': []
    }

    # 1. 检查 logo.png
    logo_path = os.path.join(dir_path, 'logo.png')
    result['logo_exists'] = os.path.isfile(logo_path)

    # 2. 检查 config.json
    config_path = os.path.join(dir_path, 'config.json')
    if not os.path.isfile(config_path):
        result['config_exists'] = False
        result['errors'].append('config.json 文件缺失')
        return result
    result['config_exists'] = True

    try:
        with open(config_path, 'r', encoding='utf-8') as f:
            config = json.load(f)
    except Exception as e:
        result['errors'].append(f'config.json 解析失败: {e}')
        return result

    # 3. 校验 name 是否与文件夹名一致
    name = config.get('name')
    if name is None:
        result['errors'].append('name 字段缺失')
    else:
        if name == dir_name:
            result['name_match'] = True
        else:
            result['errors'].append(f'name 不匹配: "{name}" ≠ "{dir_name}"')

    # 4. 校验 author 和 source 是否为空
    author = config.get('author')
    if author is None or author == '':
        result['author_empty'] = True
        result['errors'].append('author 为空')
    else:
        result['author_empty'] = False

    source = config.get('source')
    if source is None or source == '':
        result['source_empty'] = True
        result['errors'].append('source 为空')
    else:
        result['source_empty'] = False

    # 5. 校验 cursor 中每个非空文件名对应的文件是否存在
    cursor = config.get('cursor')
    if cursor is None:
        result['errors'].append('cursor 字段缺失')
    elif not isinstance(cursor, dict):
        result['errors'].append('cursor 不是字典')
    else:
        for key, filename in cursor.items():
            if filename and isinstance(filename, str) and filename.strip():
                file_path = os.path.join(dir_path, filename)
                if not os.path.isfile(file_path):
                    result['cursor_files_exist'] = False
                    result['errors'].append(f'cursor 文件缺失: {filename} (键: {key})')

    return result


def main():
    # 获取当前工作目录（脚本运行目录）
    base_dir = os.getcwd()

    # 获取所有一级子文件夹（排除文件）
    subdirs = [
        d for d in os.listdir(base_dir)
        if os.path.isdir(os.path.join(base_dir, d))
    ]
    total = len(subdirs)
    if total == 0:
        print("当前目录下没有子文件夹。")
        return

    print(f"共发现 {total} 个子文件夹，开始校验...\n")

    results = []
    for d in subdirs:
        dir_path = os.path.join(base_dir, d)
        res = check_directory(dir_path, d)
        results.append(res)

    # ---------- 统计汇总 ----------
    logo_count = sum(1 for r in results if r['logo_exists'])
    config_count = sum(1 for r in results if r['config_exists'])
    name_match_count = sum(1 for r in results if r['name_match'])
    cursor_ok_count = sum(1 for r in results if r['cursor_files_exist'])
    author_nonempty = sum(1 for r in results if not r['author_empty'])
    source_nonempty = sum(1 for r in results if not r['source_empty'])
    no_errors = sum(1 for r in results if not r['errors'])

    print("========== 统计报告 ==========")
    print(f"总文件夹数         : {total}")
    print(f"logo.png 存在      : {logo_count}")
    print(f"config.json 存在   : {config_count}")
    print(f"name 匹配          : {name_match_count}")
    print(f"cursor 文件全部存在: {cursor_ok_count}")
    print(f"author 非空        : {author_nonempty}")
    print(f"source 非空        : {source_nonempty}")
    print(f"完全通过校验的文件夹: {no_errors}")
    print("==============================\n")

    # 详细错误信息
    error_dirs = [r for r in results if r['errors']]
    if error_dirs:
        print("以下文件夹存在问题：")
        for r in error_dirs:
            print(f"\n📁 {r['dir']}")
            for err in r['errors']:
                print(f"   ❌ {err}")
    else:
        print("🎉 所有文件夹均通过校验！")


if __name__ == "__main__":
    main()