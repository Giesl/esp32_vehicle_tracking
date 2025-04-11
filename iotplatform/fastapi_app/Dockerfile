# 
FROM python:3.9

# 
WORKDIR /code

# 
COPY ./requirements.txt /code/requirements.txt

# 
RUN pip install --no-cache-dir --upgrade -r /code/requirements.txt

# 
COPY ./app /code/fastapi_app/app

# 
CMD ["uvicorn", "fastapi_app.app.main:app", "--host", "0.0.0.0", "--port", "8000"]