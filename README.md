# open_source

IoT 환경에서 동작하는 기본적인 웹서버 구현

Posix 표준 환경에서 c언어를 사용하여 간단한 IoT 시스템을 구현한다.

### 설계 목표
1. 센서 정보를 서버에 보내는 센서 프로그램을 구현한다.
2. 센서 정보를 저장하는 서버 프로그램을 구현한다.
3. 저장된 정보를 열람하는 모니터 프로그램을 구현한다.
4. 웹 서버와 클라이언트들 간 HTTP(REST protocol)을 표준으로 한다.
5. Robust I/O의 프로토콜을 사용한다.
6. 데이터베이스 (mySQL)를 사용하여 정보를 관리한다. 

사용한 기기 (라즈베리파이 + 온습도센서)

![image](https://user-images.githubusercontent.com/53904156/125301236-c0284a00-e365-11eb-911e-8a8b4b7c2bfd.png)

clientRPI 실행화면 (센서 정보 전달)

![image](https://user-images.githubusercontent.com/53904156/125301291-ce766600-e365-11eb-9ae8-ad645c953886.png)

server 실행화면

![image](https://user-images.githubusercontent.com/53904156/125301497-02518b80-e366-11eb-91c5-8419a15cbbc4.png)


clientGet 실행화면

![image](https://user-images.githubusercontent.com/53904156/125301575-13020180-e366-11eb-93fc-3b289e831ce2.png)
