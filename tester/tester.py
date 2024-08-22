import subprocess, os, sys, re


def compile() -> bool:
    os.chdir("build_make")
    result = subprocess.run(["make", "all"])
    return result.returncode == 0


def get_uid_list() -> str:
    # 環境変数で標準エンコードをUTF-8に設定
    os.environ["PYTHONIOENCODING"] = "utf-8"

    # subprocess.run()でコマンドを実行し、その結果を取得
    result = subprocess.run(
        ["pyocd", "list"], stdout=subprocess.PIPE, text=True, encoding="utf-8"
    )

    # コマンドが成功した場合、結果を処理
    lines = result.stdout.splitlines()
    uids = []
    for line in lines:
        parts = re.split(r"\s{2,}", line)
        id_index = 1
        uid_index = 3
        if len(parts) > uid_index and parts[id_index].isdigit():
            uids.append(parts[uid_index])

    return uids


def delete_program(uid: str) -> None:
    result = subprocess.run(
        ["pyocd", "erase", "--mass", "--uid", uid],
        stdout=subprocess.PIPE,
        text=True,
        encoding="utf-8",
    )


def transfer_program(uid: str) -> None:
    result = subprocess.run(
        [
            "pyocd",
            "flash",
            "-t",
            "nrf51",
            "mtkernel_3.elf",
            "--frequency",
            "1000000",
            "--uid",
            uid,
        ],
        stdout=subprocess.PIPE,
        text=True,
        encoding="utf-8",
    )


if __name__ == "__main__":
    # コンパイル失敗時はプログラムを終了
    if not compile():
        exit(1)

    # コマンドライン引数で指定されたインデックスのUnique IDを取得
    uid_list = get_uid_list()
    if len(sys.argv) > 2:
        uids = [uid_list[int(sys.argv[1])]]
    else:
        uids = uid_list

    for uid in uids:
        print("Writing Program to: " + uid)
        delete_program(uid)
        transfer_program(uid)
