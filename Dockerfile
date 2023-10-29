FROM balenalib/raspberrypi4-64-debian-python:latest

WORKDIR /night-monitor

COPY requirements.txt ./
RUN pip install --no-cache-dir -r requirements.txt

COPY . .

CMD ["sh", "start.sh"]