import subprocess

def run_main():
    subprocess.run(["uvicorn", "main:app", "--reload"])

if __name__ == "__main__":
    run_main()