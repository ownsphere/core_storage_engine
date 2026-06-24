package main

import (
	"context"
	"log"
	"net"
	"os"
	"os/signal"
	"syscall"

	apipkg "github.com/ownsphere/core_storage_engine/go/internal/api"
	"github.com/ownsphere/core_storage_engine/go/internal/api/storagepb"
	servicepkg "github.com/ownsphere/core_storage_engine/go/internal/service"
	clientpkg "github.com/ownsphere/core_storage_engine/go/pkg/client"
	"google.golang.org/grpc"
	"google.golang.org/grpc/health"
	healthpb "google.golang.org/grpc/health/grpc_health_v1"
	"google.golang.org/grpc/reflection"
)

func main() {
	addr := ":9090"
	if port := os.Getenv("PORT"); port != "" {
		addr = ":" + port
	}
	storageRoot := os.Getenv("STORAGE_ROOT")

	listener, err := net.Listen("tcp", addr)
	if err != nil {
		log.Fatal(err)
	}

	storageClient, err := clientpkg.New(storageRoot)
	if err != nil {
		log.Fatal(err)
	}
	storageService, err := servicepkg.New(storageClient)
	if err != nil {
		log.Fatal(err)
	}
	defer func() {
		if err := storageService.Close(); err != nil {
			log.Printf("storage service close error: %v", err)
		}
	}()

	apiServer, err := apipkg.NewGRPCServer(storageService)
	if err != nil {
		log.Fatal(err)
	}

	grpcServer := grpc.NewServer()
	storagepb.RegisterStorageEngineServiceServer(grpcServer, apiServer)

	healthServer := health.NewServer()
	healthpb.RegisterHealthServer(grpcServer, healthServer)
	healthServer.SetServingStatus("", healthpb.HealthCheckResponse_SERVING)

	reflection.Register(grpcServer)

	ctx, stop := signal.NotifyContext(context.Background(), syscall.SIGINT, syscall.SIGTERM)
	defer stop()

	go func() {
		<-ctx.Done()
		healthServer.SetServingStatus("", healthpb.HealthCheckResponse_NOT_SERVING)
		grpcServer.GracefulStop()
	}()

	log.Printf("OwnSphere gRPC server listening on %s", addr)
	if err := grpcServer.Serve(listener); err != nil {
		log.Fatal(err)
	}
}
