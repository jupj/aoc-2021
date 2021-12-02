//go:build ignore

package main

import (
	"bytes"
	"flag"
	"fmt"
	"io"
	"net/http"
	"net/http/cookiejar"
	"net/url"
	"os"
	"time"
)

func run() error {
	day := time.Now().Day()
	flag.IntVar(&day, "d", day, "day `number`")
	flag.Parse()

	session, err := os.ReadFile(".session")
	if err != nil {
		return err
	}
	session = bytes.TrimSpace(session)

	u, err := url.Parse(fmt.Sprintf("https://adventofcode.com/2021/day/%d/input", day))
	if err != nil {
		return err
	}

	var client http.Client
	client.Jar, err = cookiejar.New(nil)
	if err != nil {
		return err
	}

	client.Jar.SetCookies(u, []*http.Cookie{&http.Cookie{Name: "session", Value: string(session)}})

	resp, err := client.Get(u.String())
	if err != nil {
		return err
	}

	defer resp.Body.Close()

	_, err = io.Copy(os.Stdout, resp.Body)
	if err != nil {
		return err
	}
	return nil
}

func main() {
	if err := run(); err != nil {
		panic(err)
	}
}
