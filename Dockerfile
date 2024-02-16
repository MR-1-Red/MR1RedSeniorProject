FROM python:3.9
ADD test.py
CMD [“python”, “./test.py”]
